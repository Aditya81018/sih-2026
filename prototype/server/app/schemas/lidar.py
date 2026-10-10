from typing import Optional, List, Dict
from pydantic import BaseModel, Field


class LidarScanPoint(BaseModel):
    angle: int = Field(..., ge=0, le=360, description="Angle in degrees (0 to 275)")
    distance: Optional[float] = Field(None, description="Measured distance in cm (None if out of range)")
    sweep_direction: str = Field(default="CW", description="Current sweep direction (CW or CCW)")
    timestamp: Optional[int] = Field(None, description="ESP32 uptime timestamp in ms")


class LidarState(BaseModel):
    current_angle: int = Field(default=0, description="Latest scanned angle in degrees")
    current_distance: Optional[float] = Field(default=None, description="Latest measured distance in cm")
    sweep_direction: str = Field(default="CW", description="Current sweep direction (CW or CCW)")
    step_angle: int = Field(default=2, ge=1, le=10, description="Degrees stepped between pings (1, 2, 4, 5...)")
    active: bool = Field(default=True, description="Whether scanning is active")
    device_connected: bool = Field(default=False, description="Whether ESP32 hardware is connected")
    active_controllers: int = Field(default=0, description="Number of connected web controllers")
    total_points: int = Field(default=0, description="Number of points stored in current scan map")
    last_updated: Optional[str] = Field(default=None, description="ISO timestamp of last reading")


class LidarCommand(BaseModel):
    active: Optional[bool] = Field(default=None, description="Pause or resume scanning")
    reset: Optional[bool] = Field(default=None, description="Reset motor to 0 degree home position")
    step_angle: Optional[int] = Field(default=None, ge=1, le=10, description="Set degree increment (1, 2, 4, 5...)")
