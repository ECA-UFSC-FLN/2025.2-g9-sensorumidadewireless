"""
File: processes.py
Project: Estufa Dashboard API
Created: Wednesday, 29th October 2025 8:58:19 am
Author: Klaus Begnis

Copyright (c) 2025 Estufa Dashboard. All rights reserved.
"""

import datetime
from typing import Annotated

from fastapi import APIRouter, Depends, HTTPException, Response

from app.config.timezone_config import SAO_PAULO_TZ
from app.dependencies import get_db_client
from app.models.processes import CreateProcessRequest
from app.services.database.psg_client import PSGClient
from app.services.database.tables.measurements import PydanticMeasurement
from app.services.database.tables.processes import PydanticProcess

router = APIRouter(prefix="/processes", tags=["processes"])


@router.get("/")
async def get_processes(
    db_client: Annotated[PSGClient, Depends(get_db_client)],
) -> list[PydanticProcess]:
    """
    Get all processes.

    Returns:
        list[PydanticProcess]: The list of processes.
    """
    return db_client.get_all_processes()


@router.get("/{process_id}")
async def get_process_by_id(
    process_id: int,
    db_client: Annotated[PSGClient, Depends(get_db_client)],
) -> PydanticProcess:
    """
    Get a process by id.

    Args:
        process_id (int): The id of the process.

    Returns:
        PydanticProcess: The process.

    Raises:
        HTTPException: If process not found.
    """
    process = db_client.get_process_by_id(process_id)
    if not process:
        raise HTTPException(
            status_code=404,
            detail=f"Process {process_id} not found",
        )
    return process


@router.post("/start")
async def start_new_process(
    request: CreateProcessRequest,
    db_client: Annotated[PSGClient, Depends(get_db_client)],
) -> PydanticProcess:
    """
    Start a new process.

    Args:
        request (CreateProcessRequest): The request with process name.

    Returns:
        PydanticProcess: The created process.

    Raises:
        HTTPException: If process creation fails.
    """
    now = datetime.datetime.now(SAO_PAULO_TZ)
    new_process = PydanticProcess(
        id=0,  # Will be set by database
        name=request.name,
        started_at=now,
        ended_at=None,
    )
    created_process = db_client.create_new_process(new_process)
    if not created_process:
        raise HTTPException(
            status_code=500,
            detail="Failed to create process",
        )
    return created_process


@router.post("/end/{process_id}")
async def end_process(
    process_id: int,
    db_client: Annotated[PSGClient, Depends(get_db_client)],
) -> Response:
    """
    End a process.

    Args:
        process_id (int): The id of the process.

    Returns:
        Response: Success response.

    Raises:
        HTTPException: If process not found or update fails.
    """
    # Check if process exists
    process = db_client.get_process_by_id(process_id)
    if not process:
        raise HTTPException(
            status_code=404,
            detail=f"Process {process_id} not found",
        )

    success = db_client.end_process(process_id)
    if not success:
        raise HTTPException(
            status_code=500,
            detail="Failed to end process",
        )
    return Response(status_code=200)


@router.get("/{process_id}/measurements")
async def get_measurements(
    process_id: int,
    db_client: Annotated[PSGClient, Depends(get_db_client)],
) -> list[PydanticMeasurement]:
    """
    Get all measurements from a process.

    Args:
        process_id (int): The id of the process.

    Returns:
        list[PydanticMeasurement]: The list of measurements.

    Raises:
        HTTPException: If process not found.
    """
    # Check if process exists
    process = db_client.get_process_by_id(process_id)
    if not process:
        raise HTTPException(
            status_code=404,
            detail=f"Process {process_id} not found",
        )

    return db_client.get_all_measurements_from_process_id(process_id)


@router.delete("/{process_id}")
async def delete_process(
    process_id: int,
    db_client: Annotated[PSGClient, Depends(get_db_client)],
) -> Response:
    """
    Delete a process and all related data.

    Args:
        process_id (int): The id of the process to delete.

    Returns:
        Response: Success response (204 No Content).

    Raises:
        HTTPException: If process not found or deletion fails.
    """
    # Check if process exists
    process = db_client.get_process_by_id(process_id)
    if not process:
        raise HTTPException(
            status_code=404,
            detail=f"Process {process_id} not found",
        )

    success = db_client.delete_process(process_id)
    if not success:
        raise HTTPException(
            status_code=500,
            detail="Failed to delete process",
        )
    return Response(status_code=204)
