from collections.abc import AsyncGenerator
from contextlib import asynccontextmanager

from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

from .routers import health_router
from .services.database.init_db import close_database, initialize_database
from .services.database.psg_client import PSGClient


@asynccontextmanager
async def lifespan(app: FastAPI) -> AsyncGenerator:
    """
    Lifespan handler for the FastAPI application.

    Args:
        app (FastAPI): The FastAPI application instance.

    Raises:
        RuntimeError: If the database initialization fails.
    """
    # Startup
    app.state.db_client: PSGClient | None = None
    app.state.db_client = initialize_database()
    if not app.state.db_client:
        raise RuntimeError(
            "Failed to initialize database.",
        )
    yield
    # Shutdown
    if app.state.db_client:
        close_database(app.state.db_client)


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
app.include_router(health_router, prefix="/api")


def get_database_client() -> PSGClient | None:
    """
    Get the database client instance.

    Returns:
        PSGClient | None: The database client instance.
    """
    return app.state.db_client


@app.get("/")
async def root() -> dict[str, str]:
    """
    Root endpoint that returns API information.

    Returns:
        dict[str, str]: A message indicating the API name
    """
    return {"message": "Estufa Dashboard API"}
