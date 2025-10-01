from typing import Literal

from pydantic import BaseModel


class HealthResponse(BaseModel):
    """Health check response model."""

    status: Literal["ok", "error"]
    version: str
    uptime: float
