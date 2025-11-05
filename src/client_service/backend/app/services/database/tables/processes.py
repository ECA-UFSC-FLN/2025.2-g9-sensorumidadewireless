"""
File: process.py
Project: replace with your project name
Created: Wednesday, 22nd October 2025 9:29:26 am
Author: replace with your name

Copyright (c) 2025 replace with company name. All rights reserved.
"""

import datetime

from pydantic import BaseModel
from sqlalchemy import Column, DateTime, Integer, String

from app.services.database.tables.base import Base


class Process(Base):
    """Process table."""

    __tablename__ = "processes"
    id = Column(Integer, primary_key=True)
    name = Column(String)
    started_at = Column(DateTime)
    ended_at = Column(DateTime)


class PydanticProcess(BaseModel):
    id: int
    name: str
    started_at: datetime.datetime
    ended_at: datetime.datetime | None
