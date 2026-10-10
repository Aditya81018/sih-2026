from datetime import datetime, timezone
from fastapi import APIRouter
from app.core.config import settings
from app.schemas.response import APIResponse, HealthResponse

router = APIRouter()


@router.get("/health", response_model=APIResponse[HealthResponse], summary="System Health Check")
async def health_check():
    """
    Returns system status, application metadata, environment, and server timestamp.
    """
    payload = HealthResponse(
        status="ok",
        app_name=settings.PROJECT_NAME,
        version=settings.VERSION,
        environment=settings.ENV,
        timestamp=datetime.now(timezone.utc).isoformat()
    )
    return APIResponse(
        success=True,
        message="System is healthy",
        data=payload
    )
