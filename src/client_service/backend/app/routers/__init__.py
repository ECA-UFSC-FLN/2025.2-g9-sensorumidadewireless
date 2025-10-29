"""Python package init."""

from .health import router as health_router
from .processes import router as processes_router
from .sensors import router as sensors_router

__all__ = ["health_router", "processes_router", "sensors_router"]