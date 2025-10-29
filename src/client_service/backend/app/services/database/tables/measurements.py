"""
File: measurements.py
Project: replace with your project name
Created: Wednesday, 22nd October 2025 9:30:57 am
Author: replace with your name

Copyright (c) 2025 replace with company name. All rights reserved.
"""

import datetime

from pydantic import BaseModel
from sqlalchemy import (
    Column,
    DateTime,
    Float,
    ForeignKey,
    Integer,
)

from app.services.database.tables.base import Base


class Measurement(Base):
    """Measurement table."""

    __tablename__ = "measurements"
    id = Column(Integer, primary_key=True, autoincrement=True)
    process_id = Column(Integer, ForeignKey("processes.id"), nullable=False)
    sensor_id = Column(
        Integer,
        ForeignKey("sensor_registry.sensor_id"),
        nullable=False,
    )
    rh = Column(Float)
    soc = Column(Float)
    timestamp = Column(DateTime)


class PydanticMeasurement(BaseModel):
    id: int | None = None
    process_id: int
    sensor_id: int
    rh: float
    soc: float
    timestamp: datetime.datetime
