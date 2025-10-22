"""
File: init_db.py
Project: Estufa Dashboard API
Created: Wednesday, 22nd October 2025 9:24:31 am
Author: Klaus Begnis

Copyright (c) 2025 Estufa Dashboard. All rights reserved.
"""

import os
from typing import Optional

from sqlalchemy import create_engine
from sqlalchemy.exc import SQLAlchemyError

from app.services.database.psg_client import PSGClient
from app.services.database.tables.base import Base
from app.utils.logger import logger


def get_database_url() -> str:
    """
    Get database URL from environment variables.

    Returns:
        str: The database URL.
    """
    return os.getenv(
        "DATABASE_URL",
        "postgresql://estufa_user:estufa_password@localhost:5432/estufa_db",
    )


def create_database_tables() -> bool:
    """
    Create all database tables.

    Returns:
        bool: True if the tables were created successfully, False otherwise.
    """
    try:
        database_url = get_database_url()
        engine = create_engine(database_url, echo=False)

        # Criar todas as tabelas
        Base.metadata.create_all(bind=engine)

        logger.info("Successfully created all database tables")
        return True

    except SQLAlchemyError as e:
        logger.error(f"Failed to create database tables: {e}")
        return False


def initialize_database() -> Optional[PSGClient]:
    """
    Initialize database connection and create tables.

    Returns:
        Optional[PSGClient]: The database client if the initialization was
         successful, None otherwise.
    """
    try:
        # Parse database URL
        database_url = get_database_url()

        # Extract connection parameters from URL
        # Format: postgresql://user:password@host:port/database
        url_parts = database_url.replace("postgresql://", "").split("/")
        auth_part = url_parts[0]
        database = url_parts[1]

        user, password_host = auth_part.split(":")
        password, host_port = password_host.split("@")
        host, port = host_port.split(":")
        port = int(port)

        # Create PSGClient instance
        db_client = PSGClient(
            host=host,
            port=port,
            database=database,
            user=user,
            password=password,
        )

        # Connect to database
        if not db_client.connect():
            logger.error("Failed to connect to database")
            return None

        # Create tables
        if not create_database_tables():
            logger.error("Failed to create database tables")
            return None

        logger.info("Database initialization completed successfully")
        return db_client

    except Exception as e:
        logger.error(f"Failed to initialize database: {e}")
        return None


def close_database(db_client: PSGClient) -> bool:
    """
    Close database connection.

    Args:
        db_client (PSGClient): The database client to close.

    Returns:
        bool: True if the connection was closed successfully, False otherwise.
    """
    try:
        return db_client.disconnect()
    except Exception as e:
        logger.error(f"Failed to close database connection: {e}")
        return False
