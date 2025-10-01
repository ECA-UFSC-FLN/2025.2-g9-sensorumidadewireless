from collections.abc import AsyncGenerator
from contextlib import asynccontextmanager

from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

from .routers import health
from .utils.logger import logger


@asynccontextmanager
async def lifespan(_: FastAPI) -> AsyncGenerator:
    """Application lifespan handler."""
    # Startup
    logger.info("Starting up dashboard backend...")
    yield
    # Shutdown
    logger.info("Shutting down dashboard backend...")


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


@app.get("/")
async def root() -> dict[str, str]:
    """
    Root endpoint that returns API information.

    Returns:
        dict[str, str]: A message indicating the API name
    """
    return {"message": "Estufa Dashboard API"}


# TODO:
# definir banco de dados estufas - estufa id -> processos processo_id, estufa_id, medicoes -> processo_id (fk), device_id, umidade, energia
# precisa endpoint para iniciar processo (e criar nova entrada no banco de dados)
# precisa endpoint para parar processo
# precisa um worker para fazer as leituras do mqtt
# precisa de um banco de dados para por processo iniciado adicionar as medições no banco de dados
# precis
