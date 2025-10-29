"""
File: init_db.py
Project: Estufa Dashboard API
Created: Wednesday, 22nd October 2025 9:24:31 am
Author: Klaus Begnis

Copyright (c) 2025 Estufa Dashboard. All rights reserved.
"""

import os
from typing import Optional
from urllib.parse import urlparse

from sqlalchemy import create_engine, text
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
        "postgresql://root:root@localhost:5432/estufa",
    )


def create_database_if_not_exists(
    database_name: str,
    user: str,
    password: str,
    host: str,
    port: int,
) -> bool:
    """
    Create database if it doesn't exist.

    Args:
        database_name (str): Name of the database to create.
        user (str): Database user.
        password (str): Database password.
        host (str): Database host.
        port (int): Database port.

    Returns:
        bool: True if the database exists or was created successfully,
        False otherwise.
    """
    try:
        # Connect to default postgres database
        default_url = f"postgresql://{user}:{password}@{host}:{port}/postgres"
        engine = create_engine(
            default_url,
            echo=False,
            isolation_level="AUTOCOMMIT",
        )

        # Check if database exists
        with engine.connect() as conn:
            result = conn.execute(
                text(
                    "SELECT 1 FROM pg_database WHERE datname = :database_name",
                ),
                {"database_name": database_name},
            )
            exists = result.fetchone() is not None

            if not exists:
                # Create database (must be outside transaction)
                conn.execute(text(f'CREATE DATABASE "{database_name}"'))
                logger.info(f"Created database '{database_name}'")
            else:
                logger.info(f"Database '{database_name}' already exists")

        engine.dispose()
        return True

    except SQLAlchemyError as e:
        logger.error(f"Failed to create database: {e}")
        return False


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
        # Parse database URL using urllib.parse
        database_url = get_database_url()
        parsed = urlparse(database_url)

        # Extract connection parameters from parsed URL
        host = parsed.hostname or "localhost"
        port = parsed.port or 5432
        database = parsed.path.lstrip("/") or "estufa"
        user = parsed.username or "root"
        password = parsed.password or "root"

        # Create database if it doesn't exist
        if not create_database_if_not_exists(
            database,
            user,
            password,
            host,
            port,
        ):
            logger.error("Failed to create database")
            return None

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
