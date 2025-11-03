"""
File: dependencies.py
Project: Estufa Dashboard API
Created: Wednesday, 29th October 2025
Author: Klaus Begnis

Copyright (c) 2025 Estufa Dashboard. All rights reserved.
"""

from fastapi import HTTPException, Request

from app.services.database.psg_client import PSGClient
from app.services.mqtt.interfaces import IMQTTPublisher


def get_db_client(request: Request) -> PSGClient:
    """
    FastAPI dependency to get database client from app state.

    Args:
        request: FastAPI Request object (injected by dependency system).

    Returns:
        PSGClient: The database client instance.

    Raises:
        HTTPException: If database client is not available.
    """
    db_client = request.app.state.db_client
    if not db_client:
        raise HTTPException(
            status_code=500,
            detail="Database connection not available",
        )
    return db_client


def get_mqtt_publisher(request: Request) -> IMQTTPublisher:
    """
    FastAPI dependency to get MQTT publisher from app state.

    Args:
        request: FastAPI Request object (injected by dependency system).

    Returns:
        IMQTTPublisher: The MQTT publisher instance.

    Raises:
        HTTPException: If MQTT publisher is not available.
    """
    mqtt_publisher = request.app.state.mqtt_publisher
    if not mqtt_publisher:
        raise HTTPException(
            status_code=500,
            detail="MQTT publisher not available",
        )
    return mqtt_publisher
