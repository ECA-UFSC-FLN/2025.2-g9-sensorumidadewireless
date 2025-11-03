import os
from collections.abc import AsyncGenerator
from contextlib import asynccontextmanager

from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

# Importar configuração de timezone primeiro
# (deve ser feito antes de outros imports)
from .config import timezone_config  # noqa: F401
from .routers import (
    health_router,
    measurements_router,
    processes_router,
    sensors_router,
)
from .services.database.init_db import close_database, initialize_database
from .services.database.psg_client import PSGClient  # noqa: TC001
from .services.mqtt.consumer import PahoMQTTConsumer
from .services.mqtt.publisher import PahoMQTTPublisher


@asynccontextmanager
async def lifespan(app: FastAPI) -> AsyncGenerator:  # noqa: RUF029
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

    # Initialize MQTT Publisher
    # Use environment variable or default to localhost for local development
    mqtt_broker_host = os.getenv("MQTT_BROKER_HOST", "localhost")
    mqtt_broker_port = int(os.getenv("MQTT_BROKER_PORT", "1883"))

    app.state.mqtt_publisher = PahoMQTTPublisher(
        mqtt_broker_host,
        mqtt_broker_port,
    )
    if not app.state.mqtt_publisher.connect():
        raise RuntimeError("Failed to connect MQTT Publisher.")

    # Initialize MQTT Consumer
    app.state.mqtt_consumer = PahoMQTTConsumer(
        app.state.db_client,
        app.state.mqtt_publisher,
        mqtt_broker_host,
        mqtt_broker_port,
    )
    if not app.state.mqtt_consumer.connect():
        raise RuntimeError("Failed to connect MQTT Consumer.")

    # Start consumer thread
    app.state.mqtt_consumer.start()

    yield

    # Shutdown
    if hasattr(app.state, "mqtt_consumer"):
        app.state.mqtt_consumer.stop()
    if hasattr(app.state, "mqtt_publisher"):
        app.state.mqtt_publisher.disconnect()
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
app.include_router(health_router)
app.include_router(measurements_router)
app.include_router(processes_router)
app.include_router(sensors_router)


@app.get("/")
async def root() -> dict[str, str]:
    """
    Root endpoint that returns API information.

    Returns:
        dict[str, str]: A message indicating the API name
    """
    return {"message": "Estufa Dashboard API"}
