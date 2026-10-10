from fastapi import APIRouter
from app.api.v1.endpoints import arm, camera, car, health, lidar

api_router = APIRouter()

api_router.include_router(health.router, tags=["Health"])
api_router.include_router(camera.router, prefix="/camera", tags=["Camera / Sensors"])
api_router.include_router(arm.router, prefix="/arm", tags=["Robotic Arm Servo Control"])
api_router.include_router(car.router, prefix="/car", tags=["2-Wheel Robot Car Control"])
api_router.include_router(lidar.router, prefix="/lidar", tags=["275° Sonar Radar / LIDAR"])



