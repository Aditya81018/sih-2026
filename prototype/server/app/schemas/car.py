from typing import Optional
from pydantic import BaseModel, Field


class CarControlState(BaseModel):
    direction: str = Field(default="STOP", description="Current movement direction (FORWARD, BACKWARD, LEFT, RIGHT, SPIN_LEFT, SPIN_RIGHT, STOP)")
    speed: int = Field(default=150, ge=0, le=255, description="Current motor speed PWM (0 to 255)")
    device_connected: bool = Field(default=False, description="Whether ESP32 car hardware is connected")
    active_controllers: int = Field(default=0, description="Number of connected web control clients")
    last_updated: Optional[str] = Field(default=None, description="ISO timestamp of last update")


class CarCommand(BaseModel):
    direction: Optional[str] = Field(default=None, description="Target movement direction")
    speed: Optional[int] = Field(default=None, ge=0, le=255, description="Target motor speed PWM (0 to 255)")
