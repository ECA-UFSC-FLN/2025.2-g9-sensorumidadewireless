"""
File: sensor_registry.py
Project: replace with your project name
Created: Wednesday, 22nd October 2025 9:36:11 am
Author: replace with your name

Copyright (c) 2025 replace with company name. All rights reserved.
"""

from pydantic import BaseModel
from sqlalchemy import Column, ForeignKey, Integer, String

from app.services.database.tables.base import Base


class SensorRegistry(Base):
    """Sensor registry table."""

    __tablename__ = "sensor_registry"
    process_id = Column(Integer, ForeignKey("processes.id"), primary_key=True)
    sensor_id = Column(Integer, primary_key=True)
    position = Column(String)


class PydanticSensorRegistry(BaseModel):
    process_id: int
    sensor_id: int
    position: str
