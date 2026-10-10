import json
from datetime import datetime, timezone
from typing import List, Optional

from fastapi import APIRouter, HTTPException, WebSocket, WebSocketDisconnect

from app.schemas.arm import ArmCommand, ArmControlState
from app.schemas.response import APIResponse

router = APIRouter()

# =========================================================================
# Shared In-Memory Arm State (4 Servos)
# =========================================================================
_arm_state = {
    "servo1": 90,
    "servo2": 90,
    "servo3": 90,
    "servo4": 90,
    "last_updated": datetime.now(timezone.utc).isoformat(),
}

# WebSockets Connection Manager for Arm Control
class ArmConnectionManager:
    def __init__(self):
        self.device_websocket: Optional[WebSocket] = None
        self.controller_websockets: List[WebSocket] = []

    # ESP32 Device Connection
    async def connect_device(self, websocket: WebSocket):
        await websocket.accept()
        self.device_websocket = websocket
        print("🤖 [ARM WEBSOCKET] ESP32 Arm connected to /ws/device")
        # Send initial position on connect
        await self.send_to_device(_arm_state)

    def disconnect_device(self, websocket: WebSocket):
        if self.device_websocket == websocket:
            self.device_websocket = None
            print("❌ [ARM WEBSOCKET] ESP32 Arm disconnected from /ws/device")

    async def send_to_device(self, state: dict):
        if self.device_websocket:
            try:
                payload = json.dumps({
                    "servo1": state["servo1"],
                    "servo2": state["servo2"],
                    "servo3": state["servo3"],
                    "servo4": state["servo4"]
                })
                await self.device_websocket.send_text(payload)
            except Exception as e:
                print(f"⚠️ [ARM WEBSOCKET] Error sending to ESP32 arm: {e}")
                self.device_websocket = None

    # Browser Controller Connections
    async def connect_controller(self, websocket: WebSocket):
        await websocket.accept()
        self.controller_websockets.append(websocket)
        print(f"🎮 [ARM WEBSOCKET] Web controller connected. Active controllers: {len(self.controller_websockets)}")
        # Send current state immediately to newly connected slider UI
        try:
            payload = json.dumps({
                "servo1": _arm_state["servo1"],
                "servo2": _arm_state["servo2"],
                "servo3": _arm_state["servo3"],
                "servo4": _arm_state["servo4"],
                "device_connected": self.device_websocket is not None,
                "active_controllers": len(self.controller_websockets),
                "last_updated": _arm_state["last_updated"]
            })
            await websocket.send_text(payload)
        except Exception:
            pass

    def disconnect_controller(self, websocket: WebSocket):
        if websocket in self.controller_websockets:
            self.controller_websockets.remove(websocket)
            print(f"🔌 [ARM WEBSOCKET] Web controller disconnected. Active controllers: {len(self.controller_websockets)}")

    async def broadcast_to_controllers(self, sender: Optional[WebSocket] = None):
        payload = json.dumps({
            "servo1": _arm_state["servo1"],
            "servo2": _arm_state["servo2"],
            "servo3": _arm_state["servo3"],
            "servo4": _arm_state["servo4"],
            "device_connected": self.device_websocket is not None,
            "active_controllers": len(self.controller_websockets),
            "last_updated": _arm_state["last_updated"]
        })
        for connection in list(self.controller_websockets):
            if connection != sender:
                try:
                    await connection.send_text(payload)
                except Exception:
                    self.disconnect_controller(connection)


arm_manager = ArmConnectionManager()


# =========================================================================
# WEBSOCKET ENDPOINTS
# =========================================================================

@router.websocket("/ws/device")
async def esp32_arm_websocket(websocket: WebSocket):
    """
    WebSocket endpoint for ESP32 Robotic Arm hardware.
    ESP32 connects here to receive live servo angle updates (JSON).
    """
    await arm_manager.connect_device(websocket)
    await arm_manager.broadcast_to_controllers()
    try:
        while True:
            msg = await websocket.receive_text()
    except WebSocketDisconnect:
        arm_manager.disconnect_device(websocket)
        await arm_manager.broadcast_to_controllers()
    except Exception as e:
        print(f"⚠️ [ARM WEBSOCKET] Device socket error: {e}")
        arm_manager.disconnect_device(websocket)
        await arm_manager.broadcast_to_controllers()


@router.websocket("/ws/controller")
async def web_controller_websocket(websocket: WebSocket):
    """
    WebSocket endpoint for Browser Web UI controllers.
    Web UI sends live slider changes here in real time.
    """
    await arm_manager.connect_controller(websocket)
    try:
        while True:
            data = await websocket.receive_text()
            if not data:
                continue

            try:
                command = json.loads(data)
                updated = False
                if "servo1" in command and command["servo1"] is not None:
                    _arm_state["servo1"] = max(0, min(180, int(command["servo1"])))
                    updated = True

                if "servo2" in command and command["servo2"] is not None:
                    _arm_state["servo2"] = max(0, min(180, int(command["servo2"])))
                    updated = True

                if "servo3" in command and command["servo3"] is not None:
                    _arm_state["servo3"] = max(0, min(180, int(command["servo3"])))
                    updated = True

                if "servo4" in command and command["servo4"] is not None:
                    _arm_state["servo4"] = max(0, min(180, int(command["servo4"])))
                    updated = True

                if updated:
                    _arm_state["last_updated"] = datetime.now(timezone.utc).isoformat()
                    # Forward to ESP32 device
                    await arm_manager.send_to_device(_arm_state)
                    # Sync other open controller browsers
                    await arm_manager.broadcast_to_controllers(sender=websocket)

            except (json.JSONDecodeError, ValueError) as err:
                print(f"⚠️ [ARM WEBSOCKET] Invalid payload from controller: {err}")

    except WebSocketDisconnect:
        arm_manager.disconnect_controller(websocket)
        await arm_manager.broadcast_to_controllers()
    except Exception as e:
        print(f"⚠️ [ARM WEBSOCKET] Controller socket error: {e}")
        arm_manager.disconnect_controller(websocket)
        await arm_manager.broadcast_to_controllers()


# =========================================================================
# REST ENDPOINTS
# =========================================================================

@router.get("/status", response_model=APIResponse[ArmControlState], summary="Get Arm Status & Servo Angles")
async def get_arm_status():
    """Returns current 4 servo positions and connection telemetry."""
    state = ArmControlState(
        servo1=_arm_state["servo1"],
        servo2=_arm_state["servo2"],
        servo3=_arm_state["servo3"],
        servo4=_arm_state["servo4"],
        device_connected=arm_manager.device_websocket is not None,
        active_controllers=len(arm_manager.controller_websockets),
        last_updated=_arm_state["last_updated"],
    )
    return APIResponse(
        success=True,
        message=f"S1: {_arm_state['servo1']}° | S2: {_arm_state['servo2']}° | S3: {_arm_state['servo3']}° | S4: {_arm_state['servo4']}°",
        data=state
    )


@router.post("/control", response_model=APIResponse[ArmControlState], summary="Set Arm Servo Angles via REST HTTP")
async def set_arm_angles(cmd: ArmCommand):
    """
    REST fallback to set Servo 1, Servo 2, Servo 3, and/or Servo 4 angles.
    """
    updated = False
    if cmd.servo1 is not None:
        _arm_state["servo1"] = max(0, min(180, cmd.servo1))
        updated = True
    if cmd.servo2 is not None:
        _arm_state["servo2"] = max(0, min(180, cmd.servo2))
        updated = True
    if cmd.servo3 is not None:
        _arm_state["servo3"] = max(0, min(180, cmd.servo3))
        updated = True
    if cmd.servo4 is not None:
        _arm_state["servo4"] = max(0, min(180, cmd.servo4))
        updated = True

    if updated:
        _arm_state["last_updated"] = datetime.now(timezone.utc).isoformat()
        await arm_manager.send_to_device(_arm_state)
        await arm_manager.broadcast_to_controllers()

    state = ArmControlState(
        servo1=_arm_state["servo1"],
        servo2=_arm_state["servo2"],
        servo3=_arm_state["servo3"],
        servo4=_arm_state["servo4"],
        device_connected=arm_manager.device_websocket is not None,
        active_controllers=len(arm_manager.controller_websockets),
        last_updated=_arm_state["last_updated"],
    )
    return APIResponse(
        success=True,
        message="Arm positions updated successfully",
        data=state
    )
