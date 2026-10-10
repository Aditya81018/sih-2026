import asyncio
import time
from datetime import datetime, timezone
from typing import List, Optional

from fastapi import APIRouter, HTTPException, Request, WebSocket, WebSocketDisconnect
from fastapi.responses import Response, StreamingResponse

from app.schemas.camera import FrameUploadResponse, StreamStatus
from app.schemas.response import APIResponse

router = APIRouter()

# =========================================================================
# Shared In-Memory Stream State
# =========================================================================
_latest_frame_bytes: bytes = b""
_frame_event = asyncio.Event()

_stream_state = {
    "device_id": "esp32-cam-01",
    "is_active": False,
    "frame_count": 0,
    "last_frame_timestamp": None,
    "fps": 0.0,
    "last_fps_calc_time": time.time(),
    "fps_counter": 0,
    "connection_type": "None",
}

# Connection manager for browser WebSocket clients
class ConnectionManager:
    def __init__(self):
        self.active_connections: List[WebSocket] = []

    async def connect(self, websocket: WebSocket):
        await websocket.accept()
        self.active_connections.append(websocket)

    def disconnect(self, websocket: WebSocket):
        if websocket in self.active_connections:
            self.active_connections.remove(websocket)

    async def broadcast_bytes(self, data: bytes):
        for connection in list(self.active_connections):
            try:
                await connection.send_bytes(data)
            except Exception:
                self.disconnect(connection)

manager = ConnectionManager()


def _update_fps():
    global _stream_state
    _stream_state["frame_count"] += 1
    _stream_state["fps_counter"] += 1
    now = time.time()
    elapsed = now - _stream_state["last_fps_calc_time"]
    if elapsed >= 1.0:
        _stream_state["fps"] = round(_stream_state["fps_counter"] / elapsed, 1)
        _stream_state["fps_counter"] = 0
        _stream_state["last_fps_calc_time"] = now
    _stream_state["is_active"] = True
    _stream_state["last_frame_timestamp"] = datetime.now(timezone.utc).isoformat()


# =========================================================================
# WEBSOCKET INGEST: ESP32-CAM streams binary JPEG frames directly to server
# =========================================================================
@router.websocket("/ws/ingest")
async def esp32_websocket_ingest(websocket: WebSocket):
    """
    WebSocket endpoint for ESP32-CAM to push continuous real-time binary JPEG frames.
    High-efficiency streaming over persistent WebSocket connection.
    """
    global _latest_frame_bytes
    await websocket.accept()
    _stream_state["is_active"] = True
    _stream_state["connection_type"] = "WebSocket Ingest"
    print("⚡ [WEBSOCKET] ESP32-CAM connected to /ws/ingest")

    try:
        while True:
            # Receive binary frame payload from ESP32
            data = await websocket.receive_bytes()
            if not data:
                continue

            _latest_frame_bytes = data
            _frame_event.set()
            _update_fps()

            # Broadcast frame instantly to connected web browsers
            if manager.active_connections:
                asyncio.create_task(manager.broadcast_bytes(data))

    except WebSocketDisconnect:
        print("❌ [WEBSOCKET] ESP32-CAM disconnected from /ws/ingest")
    except Exception as e:
        print(f"⚠️ [WEBSOCKET] Ingest error: {e}")
    finally:
        _stream_state["is_active"] = False
        _stream_state["connection_type"] = "Disconnected"


# =========================================================================
# WEBSOCKET BROADCAST: Web browser clients receive live frames
# =========================================================================
@router.websocket("/ws")
async def camera_websocket_client(websocket: WebSocket):
    """
    WebSocket endpoint for web browser clients to receive live binary JPEG frames.
    """
    await manager.connect(websocket)
    try:
        if _latest_frame_bytes:
            await websocket.send_bytes(_latest_frame_bytes)
        while True:
            # Keep client socket alive
            await websocket.receive_text()
    except WebSocketDisconnect:
        manager.disconnect(websocket)
    except Exception:
        manager.disconnect(websocket)


# =========================================================================
# MJPEG STREAM (Fallback / Standard HTML <img> tag streaming)
# =========================================================================
async def _generate_mjpeg_stream():
    """Event-driven MJPEG generator serving incoming WebSocket frames."""
    while True:
        try:
            await asyncio.wait_for(_frame_event.wait(), timeout=1.0)
            _frame_event.clear()
        except asyncio.TimeoutError:
            pass

        if _latest_frame_bytes:
            yield (
                b"--frame\r\n"
                b"Content-Type: image/jpeg\r\n"
                b"Content-Length: " + str(len(_latest_frame_bytes)).encode() + b"\r\n\r\n"
                + _latest_frame_bytes
                + b"\r\n"
            )


@router.get("/stream", summary="Live MJPEG Camera Stream")
async def live_mjpeg_stream():
    """
    Serves live HTTP MJPEG stream (works natively in any <img> src).
    """
    return StreamingResponse(
        _generate_mjpeg_stream(),
        media_type="multipart/x-mixed-replace; boundary=frame"
    )


# =========================================================================
# RAW HTTP UPLOAD ENDPOINT (Alternative / Legacy support)
# =========================================================================
@router.post("/upload_raw", summary="High-Speed Raw Binary Frame Upload")
async def upload_raw_frame(request: Request):
    """
    HTTP POST upload fallback for frames.
    """
    global _latest_frame_bytes

    contents = await request.body()
    if not contents:
        raise HTTPException(status_code=400, detail="Empty frame body.")

    _latest_frame_bytes = contents
    _frame_event.set()
    _stream_state["connection_type"] = "HTTP Upload"
    _update_fps()

    if manager.active_connections:
        asyncio.create_task(manager.broadcast_bytes(contents))

    return Response(content="OK", media_type="text/plain", status_code=200)


# =========================================================================
# STATUS & LATEST FRAME UTILITIES
# =========================================================================
@router.get("/status", response_model=APIResponse[StreamStatus], summary="Get Camera Stream Status")
async def get_stream_status():
    """Returns current status of camera stream and FPS."""
    status = StreamStatus(
        device_id=_stream_state["device_id"],
        is_active=_stream_state["is_active"],
        frame_count=_stream_state["frame_count"],
        last_frame_timestamp=_stream_state["last_frame_timestamp"],
    )
    return APIResponse(
        success=True,
        message=f"FPS: {_stream_state['fps']} | Mode: {_stream_state['connection_type']}",
        data=status
    )


@router.get("/latest", summary="Get Latest Camera Frame")
async def get_latest_frame():
    """Returns the latest uploaded camera frame directly from RAM."""
    if not _latest_frame_bytes:
        raise HTTPException(status_code=404, detail="No camera frames received yet.")
    return Response(content=_latest_frame_bytes, media_type="image/jpeg")
