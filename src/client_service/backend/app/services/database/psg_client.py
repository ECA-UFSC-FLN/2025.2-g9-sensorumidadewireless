"""
File: psg_client.py
Project: Estufa Dashboard API
Created: Wednesday, 22nd October 2025 9:25:27 am
Author: Klaus Begnis

Copyright (c) 2025 Estufa Dashboard. All rights reserved.
"""

from sqlalchemy import create_engine, text
from sqlalchemy.exc import SQLAlchemyError
from sqlalchemy.orm import Session, sessionmaker

from app.services.database.base_client._dbclient import IDBClient
from app.services.database.tables.measurements import (
    Measurement,
    PydanticMeasurement,
)
from app.services.database.tables.processes import Process, PydanticProcess
from app.services.database.tables.sensor_registry import (
    PydanticSensorRegistry,
    SensorRegistry,
)
from app.utils.logger import logger


class PSGClient(IDBClient):
    def __init__(
        self, host: str, port: int, database: str, user: str, password: str
    ):
        self.host = host
        self.port = port
        self.database = database
        self.user = user
        self.password = password
        self.engine = None
        self.SessionLocal = None
        self._session = None

    def connect(self) -> bool:
        """
        Connect to the database.

        Returns:
            bool: True if the connection was successful, False otherwise.
        """
        try:
            # Criar URL de conexão
            database_url = f"postgresql://{self.user}:{self.password}@{self.host}:{self.port}/{self.database}"

            # Criar engine
            self.engine = create_engine(database_url, echo=False)

            # Criar SessionLocal
            self.SessionLocal = sessionmaker(
                autocommit=False, autoflush=False, bind=self.engine
            )

            # Testar conexão
            with self.engine.connect() as conn:
                conn.execute(text("SELECT 1"))

            logger.info("Successfully connected to PostgreSQL database")
            return True

        except SQLAlchemyError as e:
            logger.error(f"Failed to connect to database: {e}")
            return False

    def disconnect(self) -> bool:
        """
        Disconnect from the database.

        Returns:
            bool: True if the disconnection was successful, False otherwise.
        """
        try:
            if self._session:
                self._session.close()
            if self.engine:
                self.engine.dispose()
            logger.info("Disconnected from PostgreSQL database")
            return True
        except Exception as e:
            logger.error(f"Error disconnecting from database: {e}")
            return False

    def execute(self, query: str) -> bool:
        """
        Execute a query on the database.

        Args:
            query (str): The query to execute.

        Returns:
            bool: True if the query was executed successfully, False otherwise.
        """
        try:
            with self.engine.connect() as conn:
                conn.execute(text(query))
                conn.commit()
            return True
        except SQLAlchemyError as e:
            logger.error(f"Failed to execute query: {e}")
            return False

    def get_session(self) -> Session:
        """
        Get a database session.

        Returns:
            Session: The database session.
        """
        if not self._session:
            self._session = self.SessionLocal()
        return self._session

    def add_new_measurement(self, measurement: PydanticMeasurement) -> bool:
        """
        Add a new measurement to the database.

        Args:
            measurement (PydanticMeasurement): The measurement to add.

        Returns:
            bool: True if the measurement was added successfully, False otherwise.
        """
        try:
            session = self.get_session()
            db_measurement = Measurement(
                process_id=measurement.process_id,
                sensor_id=measurement.sensor_id,
                rh=measurement.rh,
                soc=measurement.soc,
                timestamp=measurement.timestamp,
            )
            session.add(db_measurement)
            session.commit()
            logger.info(
                f"Added new measurement for process {measurement.process_id}, "
                f"sensor {measurement.sensor_id}",
            )
            return True
        except SQLAlchemyError as e:
            logger.error(f"Failed to add measurement: {e}")
            session.rollback()
            return False

    def get_all_measurements_from_process_id(
        self, process_id: int
    ) -> list[PydanticMeasurement]:
        """
        Get all measurements from a process id.

        Args:
            process_id (int): The id of the process.

        Returns:
            list[PydanticMeasurement]: The list of measurements.
        """
        try:
            session = self.get_session()
            measurements = (
                session.query(Measurement)
                .filter(Measurement.process_id == process_id)
                .all()
            )
            return [
                PydanticMeasurement(
                    id=m.id,
                    process_id=m.process_id,
                    sensor_id=m.sensor_id,
                    rh=m.rh,
                    soc=m.soc,
                    timestamp=m.timestamp,
                )
                for m in measurements
            ]
        except SQLAlchemyError as e:
            logger.error(
                f"Failed to get measurements for process {process_id}: {e}"
            )
            return []

    def get_all_measurements_from_sensor_id(
        self,
        sensor_id: int,
    ) -> list[PydanticMeasurement]:
        """
        Get all measurements from a sensor id.

        Args:
            sensor_id (int): The id of the sensor.

        Returns:
            list[PydanticMeasurement]: The list of measurements.
        """
        try:
            session = self.get_session()
            measurements = (
                session.query(Measurement)
                .filter(Measurement.sensor_id == sensor_id)
                .all()
            )
            return [
                PydanticMeasurement(
                    id=m.id,
                    process_id=m.process_id,
                    sensor_id=m.sensor_id,
                    rh=m.rh,
                    soc=m.soc,
                    timestamp=m.timestamp,
                )
                for m in measurements
            ]
        except SQLAlchemyError as e:
            logger.error(
                f"Failed to get measurements for sensor {sensor_id}: {e}"
            )
            return []

    def create_new_process(self, process: PydanticProcess) -> bool:
        """Create a new process.

        Args:
            process (PydanticProcess): The process to create.

        Returns:
            bool: True if the process was created successfully, False otherwise.
        """
        try:
            session = self.get_session()
            db_process = Process(
                name=process.name,
                started_at=process.started_at,
                ended_at=process.ended_at,
            )
            session.add(db_process)
            session.commit()
            logger.info(f"Created new process: {process.name}")
            return True
        except SQLAlchemyError as e:
            logger.error(f"Failed to create process: {e}")
            session.rollback()
            return False

    def get_process_by_id(self, process_id: int) -> PydanticProcess:
        """Get a process by id.

        Args:
            process_id (int): The id of the process.

        Returns:
            PydanticProcess: The process.
        """
        try:
            session = self.get_session()
            process = (
                session.query(Process).filter(Process.id == process_id).first()
            )
            if process:
                return PydanticProcess(
                    id=process.id,
                    name=process.name,
                    started_at=process.started_at,
                    ended_at=process.ended_at,
                )
            return None
        except SQLAlchemyError as e:
            logger.error(f"Failed to get process {process_id}: {e}")
            return None

    def register_new_sensor(self, sensor_id: int, process_id: int) -> bool:
        """Register a new sensor.

        Args:
            sensor_id (int): The id of the sensor.
            process_id (int): The id of the process.

        Returns:
            bool: True if the sensor was registered successfully, False otherwise.
        """
        try:
            session = self.get_session()
            db_sensor = SensorRegistry(
                process_id=process_id,
                sensor_id=sensor_id,
                position="unknown",  # Default position
            )
            session.add(db_sensor)
            session.commit()
            logger.info(
                f"Registered sensor {sensor_id} for process {process_id}"
            )
            return True
        except SQLAlchemyError as e:
            logger.error(f"Failed to register sensor: {e}")
            session.rollback()
            return False

    def get_all_sensors_from_process_id(
        self,
        process_id: int,
    ) -> list[PydanticSensorRegistry]:
        """Get all sensors from a process id.

        Args:
            process_id (int): The id of the process.

        Returns:
            list[PydanticSensorRegistry]: The list of sensors.
        """
        try:
            session = self.get_session()
            sensors = (
                session.query(SensorRegistry)
                .filter(SensorRegistry.process_id == process_id)
                .all()
            )
            return [
                PydanticSensorRegistry(
                    process_id=s.process_id,
                    sensor_id=s.sensor_id,
                    position=s.position,
                )
                for s in sensors
            ]
        except SQLAlchemyError as e:
            logger.error(f"Failed to get sensors for process {process_id}: {e}")
            return []
