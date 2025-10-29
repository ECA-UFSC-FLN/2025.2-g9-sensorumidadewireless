"""
File: dependencies.py
Project: Estufa Dashboard API
Created: Wednesday, 29th October 2025
Author: Klaus Begnis

Copyright (c) 2025 Estufa Dashboard. All rights reserved.
"""

from fastapi import HTTPException, Request

from app.services.database.psg_client import PSGClient


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
