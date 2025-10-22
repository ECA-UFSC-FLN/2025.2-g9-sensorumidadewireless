"""
File: _dbclient.py
Project: replace with your project name
Created: Wednesday, 22nd October 2025 9:24:31 am
Author: replace with your name

Copyright (c) 2025 replace with company name. All rights reserved.
"""

from abc import ABC, abstractmethod

from app.services.database.tables.measurements import PydanticMeasurement
from app.services.database.tables.processes import PydanticProcess
from app.services.database.tables.sensor_registry import PydanticSensorRegistry


class IDBClient(ABC):
    @abstractmethod
    def connect(self) -> bool:
        """Connect to the database."""

    @abstractmethod
    def disconnect(self) -> bool:
        """Disconnect from the database."""

    @abstractmethod
    def execute(self, query: str) -> bool:
        """Execute a query on the database."""

    @abstractmethod
    def add_new_measurement(self, measurement: PydanticMeasurement) -> bool:
        """Add a new measurement to the database."""

    @abstractmethod
    def get_all_measurements_from_process_id(
        self, process_id: int,
    ) -> list[PydanticMeasurement]:
        """Get all measurements from a process id."""

    @abstractmethod
    def get_all_measurements_from_sensor_id(
        self, sensor_id: int,
    ) -> list[PydanticMeasurement]:
        """Get all measurements from a sensor id."""

    @abstractmethod
    def create_new_process(self, process: PydanticProcess) -> bool:
        """Create a new process."""

    @abstractmethod
    def get_process_by_id(self, process_id: int) -> PydanticProcess:
        """Get a process by id."""

    @abstractmethod
    def register_new_sensor(self, sensor_id: int, process_id: int) -> bool:
        """Register a new sensor."""

    @abstractmethod
    def get_all_sensors_from_process_id(
        self, process_id: int,
    ) -> list[PydanticSensorRegistry]:
        """Get all sensors from a process id."""
