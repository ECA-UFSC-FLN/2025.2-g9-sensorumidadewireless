"""
File: sensors.py
Project: Estufa Dashboard API
Created: Wednesday, 29th October 2025 9:01:01 am
Author: Klaus Begnis

Copyright (c) 2025 Estufa Dashboard. All rights reserved.
"""

import datetime

from fastapi import APIRouter

from app.config.timezone_config import SAO_PAULO_TZ
from app.services.database.tables.measurements import PydanticMeasurement
from app.services.database.tables.sensor_registry import PydanticSensorRegistry

router = APIRouter(prefix="/sensors", tags=["sensors"])


@router.get("/{sensor_id}/measurements")
async def get_measurements_by_sensor_id(
    sensor_id: int,
) -> list[PydanticMeasurement]:
    """
    Get all measurements from a sensor.

    Args:
        sensor_id (int): The id of the sensor.

    Returns:
        list[PydanticMeasurement]: The list of measurements.
    """
    return [
        PydanticMeasurement(
            id=1,
            process_id=1,
            sensor_id=1,
            rh=1,
            soc=1,
            timestamp=datetime.datetime.now(SAO_PAULO_TZ),
        ),
    ]


@router.get("/{sensor_id}")
async def get_sensor_by_id(sensor_id: int) -> PydanticSensorRegistry:
    """
    Get a sensor by id.

    Args:
        sensor_id (int): The id of the sensor.

    Returns:
        PydanticSensorRegistry: The sensor.
    """
    return PydanticSensorRegistry(
        process_id=1, sensor_id=1, position="Position 1",
    )
