import time

from fastapi import APIRouter

from app.models.health import HealthResponse
from app.utils.logger import logger

router = APIRouter(tags=["health"])
start_time = time.time()


@router.get("/health")
async def health_check() -> HealthResponse:
    """Health check endpoint.

    Returns:
        HealthResponse: The current health status of the service.
    """
    uptime = time.time() - start_time
    logger.debug(f"Health check request received. Uptime: {uptime:.2f}s")
    return HealthResponse(
        status="ok",
        version="0.1.0",
        uptime=uptime,
    )
