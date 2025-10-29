"""
File: processes.py
Project: Estufa Dashboard API
Created: Wednesday, 29th October 2025 8:58:19 am
Author: Klaus Begnis

Copyright (c) 2025 Estufa Dashboard. All rights reserved.
"""

import datetime

from fastapi import APIRouter, Response

from app.config.timezone_config import SAO_PAULO_TZ
from app.models.processes import CreateProcessRequest
from app.services.database.tables.measurements import PydanticMeasurement
from app.services.database.tables.processes import PydanticProcess

router = APIRouter(prefix="/processes", tags=["processes"])


@router.get("/")
async def get_processes() -> list[PydanticProcess]:
    """
    Get all processes.

    Returns:
        list[PydanticProcess]: The list of processes.
    """
    return [
        PydanticProcess(
            id=1,
            name="Process 1",
            started_at=datetime.datetime.now(SAO_PAULO_TZ),
            ended_at=datetime.datetime.now(SAO_PAULO_TZ),
        ),
    ]


@router.get("/{process_id}")
async def get_process_by_id(process_id: int) -> PydanticProcess:
    """
    Get a process by id.

    Args:
        process_id (int): The id of the process.

    Returns:
        PydanticProcess: The process.
    """
    return PydanticProcess(
        id=1,
        name="Process 1",
        started_at=datetime.datetime.now(SAO_PAULO_TZ),
        ended_at=datetime.datetime.now(SAO_PAULO_TZ),
    )


@router.post("/start")
async def start_new_process(process_name: str) -> PydanticProcess:
    """
    Start a new process.

    Args:
        process_name (str): The name of the process.

    Returns:
        PydanticProcess: The process.
    """
    return PydanticProcess(
        id=1,
        name="Process 1",
        started_at=datetime.datetime.now(SAO_PAULO_TZ),
        ended_at=datetime.datetime.now(SAO_PAULO_TZ),
    )


@router.post("/end/{process_id}")
async def end_process(process_id: int) -> Response:
    """
    End a process.

    Args:
        process_id (int): The id of the process.

    Returns:
        Response: The response.
    """
    return Response(status_code=200)


@router.get("/{process_id}/measurements")
async def get_measurements(
    process_request: CreateProcessRequest,
) -> list[PydanticMeasurement]:
    """
    Get all measurements from a process.

    Args:
        process_id (int): The id of the process.

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
