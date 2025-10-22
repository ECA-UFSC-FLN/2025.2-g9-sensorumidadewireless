from collections.abc import AsyncGenerator
from contextlib import asynccontextmanager

from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

from .routers import database_example, health
from .services.database.init_db import close_database, initialize_database
from .services.database.psg_client import PSGClient
from .utils.logger import logger

db_client: PSGClient | None = None


@asynccontextmanager
async def lifespan(_: FastAPI) -> AsyncGenerator:
    """
    Lifespan handler for the FastAPI application.

    Args:
        _ (FastAPI): The FastAPI application instance.

    Raises:
        RuntimeError: If the database initialization fails.
    """
    global db_client

    # Startup
    logger.info("Starting up dashboard backend...")

    # Initialize database
    db_client = initialize_database()
    if not db_client:
        raise RuntimeError(
            "Failed to initialize database. Application may not work correctly."
        )

    yield

    # Shutdown
    logger.info("Shutting down dashboard backend...")

    # Close database connection
    if db_client:
        close_database(db_client)
        logger.info("Database connection closed")


app = FastAPI(
    title="Estufa Dashboard API",
    description="API para monitoramento de estufas",
    version="0.1.0",
    lifespan=lifespan,
)

# CORS
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Routers
app.include_router(health.router, prefix="/api")
app.include_router(database_example.router, prefix="/api")


def get_database_client() -> PSGClient | None:
    """
    Get the database client instance.

    Returns:
        PSGClient | None: The database client instance.
    """
    return db_client


@app.get("/")
async def root() -> dict[str, str]:
    """
    Root endpoint that returns API information.

    Returns:
        dict[str, str]: A message indicating the API name
    """
    return {"message": "Estufa Dashboard API"}
