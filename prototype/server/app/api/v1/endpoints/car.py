import json
from datetime import datetime, timezone
from typing import List, Optional

from fastapi import APIRouter, HTTPException, WebSocket, WebSocketDisconnect

from app.schemas.car import CarCommand, CarControlState
from app.schemas.response import APIResponse

router = APIRouter()

# =========================================================================
# Shared In-Memory Car State
# =========================================================================
_car_state = {
    "direction": "STOP",
    "speed": 150,
    "last_updated": datetime.now(timezone.utc).isoformat(),
}


# WebSockets Connection Manager for 2-Wheel Robot Car
class CarConnectionManager:
    def __init__(self):
        self.device_websocket: Optional[WebSocket] = None
        self.controller_websockets: List[WebSocket] = []

    # ESP32 Device Connection
    async def connect_device(self, websocket: WebSocket):
        await websocket.accept()
        self.device_websocket = websocket
        print("🏎️ [CAR WEBSOCKET] ESP32 Robot Car connected to /ws/device")
        # Send initial state on connect
        await self.send_to_device(_car_state)

    def disconnect_device(self, websocket: WebSocket):
        if self.device_websocket == websocket:
            self.device_websocket = None
            print("❌ [CAR WEBSOCKET] ESP32 Robot Car disconnected from /ws/device")

    async def send_to_device(self, state: dict):
        if self.device_websocket:
            try:
                payload = json.dumps({
                    "direction": state["direction"],
                    "speed": state["speed"]
                })
                await self.device_websocket.send_text(payload)
            except Exception as e:
                print(f"⚠️ [CAR WEBSOCKET] Error sending to ESP32 car: {e}")
                self.device_websocket = None

    # Browser Controller Connections
    async def connect_controller(self, websocket: WebSocket):
        await websocket.accept()
        self.controller_websockets.append(websocket)
        print(f"🎮 [CAR WEBSOCKET] Web controller connected. Active controllers: {len(self.controller_websockets)}")
        # Send current state immediately to newly connected web client
        try:
            payload = json.dumps({
                "direction": _car_state["direction"],
                "speed": _car_state["speed"],
                "device_connected": self.device_websocket is not None,
                "active_controllers": len(self.controller_websockets),
                "last_updated": _car_state["last_updated"]
            })
            await websocket.send_text(payload)
        except Exception:
            pass

    def disconnect_controller(self, websocket: WebSocket):
        if websocket in self.controller_websockets:
            self.controller_websockets.remove(websocket)
            print(f"🔌 [CAR WEBSOCKET] Web controller disconnected. Active controllers: {len(self.controller_websockets)}")

    async def broadcast_to_controllers(self, sender: Optional[WebSocket] = None):
        payload = json.dumps({
            "direction": _car_state["direction"],
            "speed": _car_state["speed"],
            "device_connected": self.device_websocket is not None,
            "active_controllers": len(self.controller_websockets),
            "last_updated": _car_state["last_updated"]
        })
        for connection in list(self.controller_websockets):
            if connection != sender:
                try:
                    await connection.send_text(payload)
                except Exception:
                    self.disconnect_controller(connection)


car_manager = CarConnectionManager()


# =========================================================================
# WEBSOCKET ENDPOINTS
# =========================================================================

@router.websocket("/ws/device")
async def esp32_car_websocket(websocket: WebSocket):
    """
    WebSocket endpoint for ESP32 Robot Car hardware.
    ESP32 connects here to receive live movement & speed updates (JSON).
    """
    await car_manager.connect_device(websocket)
    await car_manager.broadcast_to_controllers()
    try:
        while True:
            msg = await websocket.receive_text()
    except WebSocketDisconnect:
        car_manager.disconnect_device(websocket)
        await car_manager.broadcast_to_controllers()
    except Exception as e:
        print(f"⚠️ [CAR WEBSOCKET] Device socket error: {e}")
        car_manager.disconnect_device(websocket)
        await car_manager.broadcast_to_controllers()


@router.websocket("/ws/controller")
async def web_controller_websocket(websocket: WebSocket):
    """
    WebSocket endpoint for Browser Web UI controllers.
    Web UI sends live movement/speed commands here in real time.
    """
    await car_manager.connect_controller(websocket)
    try:
        while True:
            data = await websocket.receive_text()
            if not data:
                continue

            try:
                command = json.loads(data)
                updated = False

                if "direction" in command and command["direction"] is not None:
                    _car_state["direction"] = str(command["direction"]).upper()
                    updated = True

                if "speed" in command and command["speed"] is not None:
                    _car_state["speed"] = max(0, min(255, int(command["speed"])))
                    updated = True

                if updated:
                    _car_state["last_updated"] = datetime.now(timezone.utc).isoformat()
                    # Forward to ESP32 hardware device
                    await car_manager.send_to_device(_car_state)
                    # Sync all active browser controller sessions
                    await car_manager.broadcast_to_controllers(sender=websocket)

            except (json.JSONDecodeError, ValueError) as err:
                print(f"⚠️ [CAR WEBSOCKET] Invalid payload from controller: {err}")

    except WebSocketDisconnect:
        car_manager.disconnect_controller(websocket)
        await car_manager.broadcast_to_controllers()
    except Exception as e:
        print(f"⚠️ [CAR WEBSOCKET] Controller socket error: {e}")
        car_manager.disconnect_controller(websocket)
        await car_manager.broadcast_to_controllers()


# =========================================================================
# REST ENDPOINTS
# =========================================================================

@router.get("/status", response_model=APIResponse[CarControlState], summary="Get Robot Car Motion Status & Telemetry")
async def get_car_status():
    """Returns current car movement direction, speed PWM, and connection telemetry."""
    state = CarControlState(
        direction=_car_state["direction"],
        speed=_car_state["speed"],
        device_connected=car_manager.device_websocket is not None,
        active_controllers=len(car_manager.controller_websockets),
        last_updated=_car_state["last_updated"],
    )
    return APIResponse(
        success=True,
        message=f"Direction: {_car_state['direction']} | Speed: {_car_state['speed']} PWM",
        data=state
    )


@router.post("/control", response_model=APIResponse[CarControlState], summary="Set Robot Car Movement via REST HTTP")
async def set_car_command(cmd: CarCommand):
    """
    REST fallback endpoint to set direction and/or speed.
    """
    updated = False
    if cmd.direction is not None:
        _car_state["direction"] = cmd.direction.upper()
        updated = True

    if cmd.speed is not None:
        _car_state["speed"] = max(0, min(255, cmd.speed))
        updated = True

    if updated:
        _car_state["last_updated"] = datetime.now(timezone.utc).isoformat()
        await car_manager.send_to_device(_car_state)
        await car_manager.broadcast_to_controllers()

    state = CarControlState(
        direction=_car_state["direction"],
        speed=_car_state["speed"],
        device_connected=car_manager.device_websocket is not None,
        active_controllers=len(car_manager.controller_websockets),
        last_updated=_car_state["last_updated"],
    )
    return APIResponse(
        success=True,
        message="Car motion state updated successfully",
        data=state
    )
