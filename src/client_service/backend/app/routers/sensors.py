"""
File: sensors.py
Project: Estufa Dashboard API
Created: Wednesday, 29th October 2025 9:01:01 am
Author: Klaus Begnis

Copyright (c) 2025 Estufa Dashboard. All rights reserved.
"""

from typing import Annotated

from fastapi import APIRouter, Depends, HTTPException, Response

from app.dependencies import get_db_client
from app.services.database.psg_client import PSGClient
from app.services.database.tables.measurements import PydanticMeasurement
from app.services.database.tables.sensor_registry import PydanticSensorRegistry

router = APIRouter(prefix="/sensors", tags=["sensors"])


@router.get("/{sensor_id}/measurements")
async def get_measurements_by_sensor_id(
    sensor_id: int,
    db_client: Annotated[PSGClient, Depends(get_db_client)],
) -> list[PydanticMeasurement]:
    """
    Get all measurements from a sensor.

    Args:
        sensor_id (int): The id of the sensor.

    Returns:
        list[PydanticMeasurement]: The list of measurements.
    """
    return db_client.get_all_measurements_from_sensor_id(sensor_id)


@router.get("/{sensor_id}")
async def get_sensor_by_id(
    sensor_id: int,
    db_client: Annotated[PSGClient, Depends(get_db_client)],
) -> PydanticSensorRegistry:
    """
    Get a sensor by id.

    Args:
        sensor_id (int): The id of the sensor.

    Returns:
        PydanticSensorRegistry: The sensor.

    Raises:
        HTTPException: If sensor not found.
    """
    sensor = db_client.get_sensor_by_id(sensor_id)
    if not sensor:
        raise HTTPException(
            status_code=404,
            detail=f"Sensor {sensor_id} not found",
        )
    return sensor


@router.delete("/{sensor_id}")
async def delete_sensor(
    sensor_id: int,
    db_client: Annotated[PSGClient, Depends(get_db_client)],
) -> Response:
    """
    Delete a sensor and all related measurements.

    Args:
        sensor_id (int): The id of the sensor to delete.

    Returns:
        Response: Success response (204 No Content).

    Raises:
        HTTPException: If sensor not found or deletion fails.
    """
    # Check if sensor exists
    sensor = db_client.get_sensor_by_id(sensor_id)
    if not sensor:
        raise HTTPException(
            status_code=404,
            detail=f"Sensor {sensor_id} not found",
        )

    success = db_client.delete_sensor(sensor_id)
    if not success:
        raise HTTPException(
            status_code=500,
            detail="Failed to delete sensor",
        )
    return Response(status_code=204)
