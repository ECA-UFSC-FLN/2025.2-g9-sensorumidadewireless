"""
File: psg_client.py
Project: Estufa Dashboard API
Created: Wednesday, 22nd October 2025 9:25:27 am
Author: Klaus Begnis

Copyright (c) 2025 Estufa Dashboard. All rights reserved.
"""

import datetime

from sqlalchemy import create_engine, text
from sqlalchemy.exc import SQLAlchemyError
from sqlalchemy.orm import Session, sessionmaker

from app.config.timezone_config import SAO_PAULO_TZ
from app.services.database.base_client._dbclient import (
    IDBClient,  # noqa: PLC2701
)
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
        self,
        host: str,
        port: int,
        database: str,
        user: str,
        password: str,
    ) -> None:
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
                autocommit=False,
                autoflush=False,
                bind=self.engine,
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
            bool: True if the measurement was added successfully,
            False otherwise.
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
        self,
        process_id: int,
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
                f"Failed to get measurements for process {process_id}: {e}",
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
                f"Failed to get measurements for sensor {sensor_id}: {e}",
            )
            return []

    def create_new_process(
        self,
        process: PydanticProcess,
    ) -> PydanticProcess | None:
        """Create a new process.

        Args:
            process (PydanticProcess): The process to create.

        Returns:
            PydanticProcess | None: The created process with ID,
            or None if failed.
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
            session.refresh(db_process)
            logger.info(f"Created new process: {process.name}")
            return PydanticProcess(
                id=db_process.id,
                name=db_process.name,
                started_at=db_process.started_at,
                ended_at=db_process.ended_at,
            )
        except SQLAlchemyError as e:
            logger.error(f"Failed to create process: {e}")
            session.rollback()
            return None

    def get_process_by_id(self, process_id: int) -> PydanticProcess | None:
        """Get a process by id.

        Args:
            process_id (int): The id of the process.

        Returns:
            PydanticProcess | None: The process, or None if not found.
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

    def get_all_processes(self) -> list[PydanticProcess]:
        """Get all processes.

        Returns:
            list[PydanticProcess]: The list of processes.
        """
        try:
            session = self.get_session()
            processes = session.query(Process).all()
            return [
                PydanticProcess(
                    id=p.id,
                    name=p.name,
                    started_at=p.started_at,
                    ended_at=p.ended_at,
                )
                for p in processes
            ]
        except SQLAlchemyError as e:
            logger.error(f"Failed to get all processes: {e}")
            return []

    def end_process(self, process_id: int) -> bool:
        """End a process by updating ended_at.

        Args:
            process_id (int): The id of the process.

        Returns:
            bool: True if the process was ended successfully, False otherwise.
        """
        try:
            session = self.get_session()
            process = (
                session.query(Process).filter(Process.id == process_id).first()
            )
            if not process:
                logger.warning(f"Process {process_id} not found")
                return False

            process.ended_at = datetime.datetime.now(SAO_PAULO_TZ)
            session.commit()
            logger.info(f"Ended process {process_id}")
            return True
        except SQLAlchemyError as e:
            logger.error(f"Failed to end process {process_id}: {e}")
            session.rollback()
            return False

    def register_new_sensor(self, sensor_id: int, process_id: int) -> bool:
        """Register a new sensor.

        Args:
            sensor_id (int): The id of the sensor.
            process_id (int): The id of the process.

        Returns:
            bool: True if the sensor was registered successfully,
            False otherwise.
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
                f"Registered sensor {sensor_id} for process {process_id}",
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

    def get_sensor_by_id(self, sensor_id: int) -> PydanticSensorRegistry | None:
        """Get a sensor by id.

        Args:
            sensor_id (int): The id of the sensor.

        Returns:
            PydanticSensorRegistry | None: The sensor, or None if not found.
        """
        try:
            session = self.get_session()
            sensor = (
                session.query(SensorRegistry)
                .filter(SensorRegistry.sensor_id == sensor_id)
                .first()
            )
            if sensor:
                return PydanticSensorRegistry(
                    process_id=sensor.process_id,
                    sensor_id=sensor.sensor_id,
                    position=sensor.position,
                )
            return None
        except SQLAlchemyError as e:
            logger.error(f"Failed to get sensor {sensor_id}: {e}")
            return None

    def delete_process(self, process_id: int) -> bool:
        """Delete a process and all related data.

        Deletes in cascade order: measurements → sensor_registry → process.

        Args:
            process_id (int): The id of the process to delete.

        Returns:
            bool: True if the process was deleted successfully,
            False otherwise.
        """
        try:
            session = self.get_session()
            # Verify process exists
            process = (
                session.query(Process).filter(Process.id == process_id).first()
            )
            if not process:
                logger.warning(f"Process {process_id} not found")
                return False

            # Delete measurements first
            session.query(Measurement).filter(
                Measurement.process_id == process_id,
            ).delete()

            # Delete sensor_registry
            session.query(SensorRegistry).filter(
                SensorRegistry.process_id == process_id,
            ).delete()

            # Delete process
            session.delete(process)
            session.commit()
            logger.info(f"Deleted process {process_id} and all related data")
            return True
        except SQLAlchemyError as e:
            logger.error(f"Failed to delete process {process_id}: {e}")
            session.rollback()
            return False

    def delete_sensor(self, sensor_id: int) -> bool:
        """Delete a sensor and all related measurements.

        Deletes in cascade order: measurements → sensor_registry.

        Args:
            sensor_id (int): The id of the sensor to delete.

        Returns:
            bool: True if the sensor was deleted successfully,
            False otherwise.
        """
        try:
            session = self.get_session()
            # Verify sensor exists
            sensor = (
                session.query(SensorRegistry)
                .filter(SensorRegistry.sensor_id == sensor_id)
                .first()
            )
            if not sensor:
                logger.warning(f"Sensor {sensor_id} not found")
                return False

            # Delete measurements first
            session.query(Measurement).filter(
                Measurement.sensor_id == sensor_id,
            ).delete()

            # Delete sensor_registry
            session.delete(sensor)
            session.commit()
            logger.info(
                f"Deleted sensor {sensor_id} and all related measurements",
            )
            return True
        except SQLAlchemyError as e:
            logger.error(f"Failed to delete sensor {sensor_id}: {e}")
            session.rollback()
            return False

    def delete_measurement(self, measurement_id: int) -> bool:
        """Delete a measurement by id.

        Args:
            measurement_id (int): The id of the measurement to delete.

        Returns:
            bool: True if the measurement was deleted successfully,
            False otherwise.
        """
        try:
            session = self.get_session()
            # Verify measurement exists
            measurement = (
                session.query(Measurement)
                .filter(Measurement.id == measurement_id)
                .first()
            )
            if not measurement:
                logger.warning(f"Measurement {measurement_id} not found")
                return False

            # Delete measurement
            session.delete(measurement)
            session.commit()
            logger.info(f"Deleted measurement {measurement_id}")
            return True
        except SQLAlchemyError as e:
            logger.error(f"Failed to delete measurement {measurement_id}: {e}")
            session.rollback()
            return False
