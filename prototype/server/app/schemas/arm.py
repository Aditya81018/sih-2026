from typing import Optional
from pydantic import BaseModel, Field


class ArmControlState(BaseModel):
    servo1: int = Field(default=90, ge=0, le=180, description="Angle for Servo 1 (0 to 180 degrees)")
    servo2: int = Field(default=90, ge=0, le=180, description="Angle for Servo 2 (0 to 180 degrees)")
    servo3: int = Field(default=90, ge=0, le=180, description="Angle for Servo 3 (0 to 180 degrees)")
    servo4: int = Field(default=90, ge=0, le=180, description="Angle for Servo 4 (0 to 180 degrees)")
    device_connected: bool = Field(default=False, description="Whether ESP32 arm is connected")
    active_controllers: int = Field(default=0, description="Number of connected web control clients")
    last_updated: Optional[str] = Field(default=None, description="ISO timestamp of last update")


class ArmCommand(BaseModel):
    servo1: Optional[int] = Field(default=None, ge=0, le=180, description="Target angle for Servo 1")
    servo2: Optional[int] = Field(default=None, ge=0, le=180, description="Target angle for Servo 2")
    servo3: Optional[int] = Field(default=None, ge=0, le=180, description="Target angle for Servo 3")
    servo4: Optional[int] = Field(default=None, ge=0, le=180, description="Target angle for Servo 4")
