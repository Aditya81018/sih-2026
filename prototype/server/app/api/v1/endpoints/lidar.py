import json
from datetime import datetime, timezone
from typing import Dict, List, Optional

from fastapi import APIRouter, HTTPException, WebSocket, WebSocketDisconnect

from app.schemas.lidar import LidarCommand, LidarScanPoint, LidarState
from app.schemas.response import APIResponse

router = APIRouter()

# =========================================================================
# Shared In-Memory LIDAR Radar State & Point Cloud Buffer (0° to 275°)
# =========================================================================
_lidar_state = {
    "current_angle": 0,
    "current_distance": None,
    "sweep_direction": "CW",
    "step_angle": 2,  # Default 2 degrees per step
    "active": True,
    "last_updated": datetime.now(timezone.utc).isoformat(),
}

# Key: angle (0 to 275), Value: {"distance": float|None, "sweep_direction": str, "updated_at": str}
_scan_map: Dict[int, dict] = {}


class LidarConnectionManager:
    def __init__(self):
        self.device_websocket: Optional[WebSocket] = None
        self.controller_websockets: List[WebSocket] = []

    # ESP32 Device Connection Management
    async def connect_device(self, websocket: WebSocket):
        await websocket.accept()
        self.device_websocket = websocket
        print("📡 [LIDAR WEBSOCKET] ESP32 Sonar Radar connected to /ws/device")
        # Send initial control command state to device
        await self.send_to_device({
            "active": _lidar_state["active"],
            "step_angle": _lidar_state["step_angle"],
            "reset": False
        })

    def disconnect_device(self, websocket: WebSocket):
        if self.device_websocket == websocket:
            self.device_websocket = None
            print("❌ [LIDAR WEBSOCKET] ESP32 Sonar Radar disconnected from /ws/device")

    async def send_to_device(self, command: dict):
        if self.device_websocket:
            try:
                payload = json.dumps(command)
                await self.device_websocket.send_text(payload)
            except Exception as e:
                print(f"⚠️ [LIDAR WEBSOCKET] Error sending command to ESP32: {e}")
                self.device_websocket = None

    # Browser Web UI Controller Connections
    async def connect_controller(self, websocket: WebSocket):
        await websocket.accept()
        self.controller_websockets.append(websocket)
        print(f"📡 [LIDAR WEBSOCKET] Web radar client connected. Active clients: {len(self.controller_websockets)}")
        
        # Send full initial scan map & scanner state to newly connected web client
        try:
            initial_payload = json.dumps({
                "type": "INIT_MAP",
                "state": {
                    "current_angle": _lidar_state["current_angle"],
                    "current_distance": _lidar_state["current_distance"],
                    "sweep_direction": _lidar_state["sweep_direction"],
                    "step_angle": _lidar_state["step_angle"],
                    "active": _lidar_state["active"],
                    "device_connected": self.device_websocket is not None,
                    "active_controllers": len(self.controller_websockets),
                    "last_updated": _lidar_state["last_updated"]
                },
                "scan_map": _scan_map
            })
            await websocket.send_text(initial_payload)
        except Exception as e:
            print(f"⚠️ [LIDAR WEBSOCKET] Failed sending init map to client: {e}")

    def disconnect_controller(self, websocket: WebSocket):
        if websocket in self.controller_websockets:
            self.controller_websockets.remove(websocket)
            print(f"🔌 [LIDAR WEBSOCKET] Web radar client disconnected. Active clients: {len(self.controller_websockets)}")

    # Broadcast single live sonar point reading to all connected web dashboards
    async def broadcast_point(self, angle: int, distance: Optional[float], sweep_direction: str):
        if not self.controller_websockets:
            return

        payload = json.dumps({
            "type": "SCAN_POINT",
            "point": {
                "angle": angle,
                "distance": distance,
                "sweep_direction": sweep_direction,
                "timestamp": datetime.now(timezone.utc).isoformat()
            },
            "device_connected": self.device_websocket is not None,
            "active_controllers": len(self.controller_websockets)
        })

        dead_clients = []
        for connection in list(self.controller_websockets):
            try:
                await connection.send_text(payload)
            except Exception:
                dead_clients.append(connection)

        for dead in dead_clients:
            if dead in self.controller_websockets:
                self.controller_websockets.remove(dead)

    # Broadcast scanner state change (e.g. pause/resume/step_angle) to all controllers
    async def broadcast_state(self):
        if not self.controller_websockets:
            return

        payload = json.dumps({
            "type": "STATE_UPDATE",
            "state": {
                "current_angle": _lidar_state["current_angle"],
                "current_distance": _lidar_state["current_distance"],
                "sweep_direction": _lidar_state["sweep_direction"],
                "step_angle": _lidar_state["step_angle"],
                "active": _lidar_state["active"],
                "device_connected": self.device_websocket is not None,
                "active_controllers": len(self.controller_websockets),
                "last_updated": _lidar_state["last_updated"]
            }
        })

        dead_clients = []
        for connection in list(self.controller_websockets):
            try:
                await connection.send_text(payload)
            except Exception:
                dead_clients.append(connection)

        for dead in dead_clients:
            if dead in self.controller_websockets:
                self.controller_websockets.remove(dead)


lidar_manager = LidarConnectionManager()


# =========================================================================
# WEBSOCKET ENDPOINTS
# =========================================================================

@router.websocket("/ws/device")
async def esp32_lidar_websocket(websocket: WebSocket):
    """
    WebSocket endpoint for ESP32 Hardware (28BYJ-48 Stepper + HC-SR04 Sonar).
    Receives live distance readings across 275° sweep range.
    """
    await lidar_manager.connect_device(websocket)
    await lidar_manager.broadcast_state()
    try:
        while True:
            raw_text = await websocket.receive_text()
            try:
                data = json.loads(raw_text)
                angle = data.get("angle", 0)
                distance = data.get("distance")  # Can be float or None
                sweep_direction = data.get("sweep_direction", "CW")

                # Update state
                now_str = datetime.now(timezone.utc).isoformat()
                _lidar_state["current_angle"] = angle
                _lidar_state["current_distance"] = distance
                _lidar_state["sweep_direction"] = sweep_direction
                _lidar_state["last_updated"] = now_str

                # Store point in map
                _scan_map[angle] = {
                    "distance": distance,
                    "sweep_direction": sweep_direction,
                    "updated_at": now_str
                }

                # Broadcast live point to all web dashboards
                await lidar_manager.broadcast_point(angle, distance, sweep_direction)

            except json.JSONDecodeError:
                pass
    except (WebSocketDisconnect, RuntimeError):
        pass
    except Exception as e:
        print(f"⚠️ [LIDAR WEBSOCKET] Device socket error: {e}")
    finally:
        lidar_manager.disconnect_device(websocket)
        await lidar_manager.broadcast_state()


@router.websocket("/ws/controller")
async def web_lidar_controller_websocket(websocket: WebSocket):
    """
    WebSocket endpoint for browser Web UI dashboards.
    Receives live radar sweep stream and sends scanner control commands.
    """
    await lidar_manager.connect_controller(websocket)
    try:
        while True:
            raw_text = await websocket.receive_text()
            try:
                data = json.loads(raw_text)
                if "active" in data:
                    _lidar_state["active"] = bool(data["active"])
                    await lidar_manager.send_to_device({"active": _lidar_state["active"]})
                if "step_angle" in data:
                    _lidar_state["step_angle"] = int(data["step_angle"])
                    await lidar_manager.send_to_device({"step_angle": _lidar_state["step_angle"]})
                if data.get("reset"):
                    await lidar_manager.send_to_device({"reset": True})
                if data.get("clear_map"):
                    _scan_map.clear()

                await lidar_manager.broadcast_state()
            except json.JSONDecodeError:
                pass
    except (WebSocketDisconnect, RuntimeError):
        pass
    except Exception as e:
        print(f"⚠️ [LIDAR WEBSOCKET] Controller socket error: {e}")
    finally:
        lidar_manager.disconnect_controller(websocket)
        await lidar_manager.broadcast_state()


# =========================================================================
# REST API ENDPOINTS
# =========================================================================

@router.get("/state", summary="Get Current LIDAR State & Summary Metrics")
async def get_lidar_state():
    valid_distances = [
        p["distance"] for p in _scan_map.values()
        if p.get("distance") is not None and p.get("distance") > 0
    ]

    min_dist = min(valid_distances) if valid_distances else None
    max_dist = max(valid_distances) if valid_distances else None

    return APIResponse(
        success=True,
        message="Current LIDAR sonar scanner state retrieved",
        data={
            "current_angle": _lidar_state["current_angle"],
            "current_distance": _lidar_state["current_distance"],
            "sweep_direction": _lidar_state["sweep_direction"],
            "step_angle": _lidar_state["step_angle"],
            "active": _lidar_state["active"],
            "device_connected": lidar_manager.device_websocket is not None,
            "active_controllers": len(lidar_manager.controller_websockets),
            "total_points": len(_scan_map),
            "nearest_obstacle_cm": min_dist,
            "farthest_obstacle_cm": max_dist,
            "sweep_range_deg": 275,
            "last_updated": _lidar_state["last_updated"]
        }
    )


@router.get("/scan", summary="Get Complete 275° Scan Map Buffer")
async def get_scan_map():
    return APIResponse(
        success=True,
        message="LIDAR 275° scan map retrieved",
        data={
            "points_count": len(_scan_map),
            "scan_map": _scan_map
        }
    )


@router.post("/control", summary="Send Control Command to Scanner")
async def control_lidar(cmd: LidarCommand):
    if cmd.active is not None:
        _lidar_state["active"] = cmd.active
        await lidar_manager.send_to_device({"active": cmd.active})

    if cmd.step_angle is not None:
        _lidar_state["step_angle"] = cmd.step_angle
        await lidar_manager.send_to_device({"step_angle": cmd.step_angle})

    if cmd.reset:
        await lidar_manager.send_to_device({"reset": True})

    await lidar_manager.broadcast_state()

    return APIResponse(
        success=True,
        message="LIDAR control command executed",
        data={
            "active": _lidar_state["active"],
            "step_angle": _lidar_state["step_angle"],
            "device_connected": lidar_manager.device_websocket is not None
        }
    )


@router.post("/clear", summary="Clear Current Scan Map History")
async def clear_scan_map():
    _scan_map.clear()
    await lidar_manager.broadcast_state()
    return APIResponse(
        success=True,
        message="LIDAR scan map cleared",
        data={"points_count": 0}
    )
