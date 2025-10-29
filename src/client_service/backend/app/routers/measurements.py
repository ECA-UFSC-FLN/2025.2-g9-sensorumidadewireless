"""
File: measurements.py
Project: Estufa Dashboard API
Created: Wednesday, 29th October 2025
Author: Klaus Begnis

Copyright (c) 2025 Estufa Dashboard. All rights reserved.
"""

from typing import Annotated

from fastapi import APIRouter, Depends, HTTPException, Response

from app.dependencies import get_db_client
from app.services.database.psg_client import PSGClient

router = APIRouter(prefix="/measurements", tags=["measurements"])


@router.delete("/{measurement_id}")
async def delete_measurement(
    measurement_id: int,
    db_client: Annotated[PSGClient, Depends(get_db_client)],
) -> Response:
    """
    Delete a measurement by id.

    Args:
        measurement_id (int): The id of the measurement to delete.

    Returns:
        Response: Success response (204 No Content).

    Raises:
        HTTPException: If measurement not found or deletion fails.
    """
    success = db_client.delete_measurement(measurement_id)
    if not success:
        raise HTTPException(
            status_code=404,
            detail=f"Measurement {measurement_id} not found",
        )
    return Response(status_code=204)
