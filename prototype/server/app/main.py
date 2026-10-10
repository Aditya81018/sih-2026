from contextlib import asynccontextmanager
from pathlib import Path
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import HTMLResponse
from fastapi.staticfiles import StaticFiles

from app.api.v1.api import api_router
from app.core.config import settings
from app.schemas.response import APIResponse

TEMPLATES_DIR = Path(__file__).parent / "templates"


@asynccontextmanager
async def lifespan(app: FastAPI):
    # Startup logic
    upload_path = Path(settings.UPLOAD_DIR)
    upload_path.mkdir(parents=True, exist_ok=True)
    print(f"🚀 [{settings.PROJECT_NAME}] Starting up in {settings.ENV} mode...")
    print(f"📁 Upload directory ensured at: {upload_path.resolve()}")
    yield
    # Shutdown logic
    print(f"🛑 [{settings.PROJECT_NAME}] Shutting down...")


app = FastAPI(
    title=settings.PROJECT_NAME,
    version=settings.VERSION,
    openapi_url=f"{settings.API_V1_STR}/openapi.json",
    docs_url="/docs",
    redoc_url="/redoc",
    lifespan=lifespan,
)

# CORS Setup
if settings.BACKEND_CORS_ORIGINS:
    app.add_middleware(
        CORSMiddleware,
        allow_origins=settings.BACKEND_CORS_ORIGINS,
        allow_credentials=True,
        allow_methods=["*"],
        allow_headers=["*"],
    )

# Static file serving for uploads
upload_dir = Path(settings.UPLOAD_DIR)
upload_dir.mkdir(parents=True, exist_ok=True)
app.mount("/uploads", StaticFiles(directory=settings.UPLOAD_DIR), name="uploads")


@app.get("/", summary="Root Landing Endpoint")
async def root():
    return APIResponse(
        success=True,
        message=f"Welcome to {settings.PROJECT_NAME} API! Visit /robot for unified robot dashboard, /car for car dashboard, /arm for arm dashboard, /lidar for 275° sonar radar dashboard, /live for camera feed, and /docs for Swagger UI documentation.",
        data={
            "name": settings.PROJECT_NAME,
            "version": settings.VERSION,
            "robot_dashboard": "/robot",
            "car_dashboard": "/car",
            "arm_dashboard": "/arm",
            "lidar_dashboard": "/lidar",
            "live_dashboard": "/live",
            "docs": "/docs",
            "redoc": "/redoc",
            "api_v1": settings.API_V1_STR,
        }
    )


@app.get("/robot", response_class=HTMLResponse, summary="Integrated Robot Controller Web UI (Car + 4-Servo Arm)")
async def robot_dashboard():
    """
    Renders the live ESP32 integrated robot controller webpage (Car + 4-Servo Arm via WebSockets).
    """
    template_path = TEMPLATES_DIR / "robot.html"
    if template_path.exists():
        with open(template_path, "r", encoding="utf-8") as f:
            return HTMLResponse(content=f.read())
    return HTMLResponse(content="<h1>Robot control dashboard template not found.</h1>", status_code=404)


@app.get("/live", response_class=HTMLResponse, summary="Live Camera Dashboard Web UI")
async def live_dashboard():
    """
    Renders the live ESP32-CAM monitoring dashboard webpage.
    """
    template_path = TEMPLATES_DIR / "live.html"
    if template_path.exists():
        with open(template_path, "r", encoding="utf-8") as f:
            return HTMLResponse(content=f.read())
    return HTMLResponse(content="<h1>Live dashboard template not found.</h1>", status_code=404)


@app.get("/arm", response_class=HTMLResponse, summary="Robotic Arm Servo Controller Web UI")
async def arm_dashboard():
    """
    Renders the live ESP32 dual servo robotic arm control webpage.
    """
    template_path = TEMPLATES_DIR / "arm.html"
    if template_path.exists():
        with open(template_path, "r", encoding="utf-8") as f:
            return HTMLResponse(content=f.read())
    return HTMLResponse(content="<h1>Arm control dashboard template not found.</h1>", status_code=404)


@app.get("/car", response_class=HTMLResponse, summary="2-Wheel Robot Car Controller Web UI")
async def car_dashboard():
    """
    Renders the live ESP32 2-wheel robot car control webpage.
    """
    template_path = TEMPLATES_DIR / "car.html"
    if template_path.exists():
        with open(template_path, "r", encoding="utf-8") as f:
            return HTMLResponse(content=f.read())
    return HTMLResponse(content="<h1>Car control dashboard template not found.</h1>", status_code=404)


@app.get("/lidar", response_class=HTMLResponse, summary="275° Sonar Radar / LIDAR Scanner Web UI")
async def lidar_dashboard():
    """
    Renders the live ESP32 275° Sonar Radar scanner webpage.
    """
    template_path = TEMPLATES_DIR / "lidar.html"
    if template_path.exists():
        with open(template_path, "r", encoding="utf-8") as f:
            return HTMLResponse(content=f.read())
    return HTMLResponse(content="<h1>LIDAR sonar radar dashboard template not found.</h1>", status_code=404)





@app.get("/health", summary="Global Health Check Shortcut")
async def root_health():
    return APIResponse(
        success=True,
        message="Server is operational",
        data={"status": "ok", "app": settings.PROJECT_NAME}
    )


# Include API v1 Router
app.include_router(api_router, prefix=settings.API_V1_STR)


if __name__ == "__main__":
    import uvicorn
    uvicorn.run(
        "app.main:app",
        host=settings.HOST,
        port=settings.PORT,
        reload=True if settings.ENV == "development" else False
    )
