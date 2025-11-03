"""
MQTT Consumer implementation using paho-mqtt.
"""

import datetime
import json
import random
import threading
import time

import paho.mqtt.client as mqtt

from app.config.timezone_config import SAO_PAULO_TZ
from app.services.database.psg_client import PSGClient
from app.services.database.tables.measurements import PydanticMeasurement
from app.services.mqtt.interfaces import IMQTTConsumer, IMQTTPublisher
from app.utils.logger import logger


class PahoMQTTConsumer(IMQTTConsumer):
    """MQTT Consumer implementation using paho-mqtt library."""

    # MQTT Topics
    TOPIC_MEASUREMENT = "sensores/medicao"
    TOPIC_BIND_REQUEST = "sensores/bind/request"
    TOPIC_BIND_RESPONSE = "sensores/bind/response"
    TOPIC_UNBIND = "sensores/bind/unbind"

    def __init__(
        self,
        db_client: PSGClient,
        publisher: IMQTTPublisher,
        broker_host: str,
        broker_port: int,
    ) -> None:
        """
        Initialize the MQTT consumer.

        Args:
            db_client (PSGClient): Database client for storing data.
            publisher (IMQTTPublisher): Publisher for sending responses.
            broker_host (str): MQTT broker hostname.
            broker_port (int): MQTT broker port.
        """
        self.db_client = db_client
        self.publisher = publisher
        self.broker_host = broker_host
        self.broker_port = broker_port
        self.client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
        self.client.on_message = self._on_message
        self._connected = False
        self._thread: threading.Thread | None = None
        self._stop_event = threading.Event()

    def connect(self) -> bool:
        """
        Connect to the MQTT broker and subscribe to topics.

        Returns:
            bool: True if connection successful, False otherwise.
        """
        try:
            self.client.connect(self.broker_host, self.broker_port, 60)
            self._connected = True

            # Subscribe to all required topics
            self.subscribe(self.TOPIC_MEASUREMENT)
            self.subscribe(self.TOPIC_BIND_REQUEST)
            self.subscribe(self.TOPIC_UNBIND)

            logger.info(
                f"MQTT Consumer connected to "
                f"{self.broker_host}:{self.broker_port}",
            )
            return True
        except Exception as e:
            logger.error(f"Failed to connect MQTT Consumer: {e}")
            self._connected = False
            return False

    def disconnect(self) -> None:
        """Disconnect from the MQTT broker."""
        if self._connected:
            self.client.disconnect()
            self._connected = False
            logger.info("MQTT Consumer disconnected")

    def subscribe(self, topic: str) -> bool:
        """
        Subscribe to a topic.

        Args:
            topic (str): The MQTT topic to subscribe to.

        Returns:
            bool: True if subscription successful, False otherwise.
        """
        try:
            result, _ = self.client.subscribe(topic)
            if result == mqtt.MQTT_ERR_SUCCESS:
                logger.info(f"Subscribed to topic: {topic}")
                return True
            logger.error(f"Failed to subscribe to {topic}: {result}")
            return False
        except Exception as e:
            logger.error(f"Error subscribing to {topic}: {e}")
            return False

    def start(self) -> None:
        """Start the consumer loop in a separate thread."""
        if self._thread is not None and self._thread.is_alive():
            logger.warning("MQTT Consumer already running")
            return

        self._stop_event.clear()
        self._thread = threading.Thread(target=self._run_loop, daemon=True)
        self._thread.start()
        logger.info("MQTT Consumer thread started")

    def stop(self) -> None:
        """Stop the consumer loop and cleanup resources."""
        self._stop_event.set()
        if self._thread is not None:
            self.client.loop_stop()
            self.disconnect()
            self._thread.join(timeout=5)
            logger.info("MQTT Consumer thread stopped")

    def _run_loop(self) -> None:
        """Internal method to run the MQTT loop."""
        try:
            self.client.loop_forever()
        except Exception as e:
            logger.error(f"Error in MQTT Consumer loop: {e}")

    def _on_message(
        self,
        client: mqtt.Client,  # noqa: ARG002
        userdata: None,  # noqa: ARG002
        msg: mqtt.MQTTMessage,
    ) -> None:
        """
        Callback when a message is received.

        Args:
            client (mqtt.Client): The MQTT client instance.
            userdata: User data (not used).
            msg (mqtt.MQTTMessage): The received message.
        """
        try:
            payload = msg.payload.decode("utf-8")
            logger.info(f"Received message on {msg.topic}: {payload}")

            if msg.topic == self.TOPIC_MEASUREMENT:
                self._handle_measurement(payload)
            elif msg.topic == self.TOPIC_BIND_REQUEST:
                self._handle_bind_request(payload)
            elif msg.topic == self.TOPIC_UNBIND:
                self._handle_unbind(payload)
            else:
                logger.warning(f"Unhandled topic: {msg.topic}")

        except Exception as e:
            logger.error(f"Error processing message: {e}")

    def _handle_measurement(self, payload: str) -> None:
        """
        Handle measurement messages.

        Expected payload: {"medicao": float, "id": str}

        Args:
            payload (str): JSON payload string.
        """
        try:
            data = json.loads(payload)
            sensor_id = int(data["id"])
            medicao = float(data["medicao"])

            # Get sensor to find active process
            sensor = self.db_client.get_sensor_by_id(sensor_id)
            if not sensor:
                logger.error(f"Sensor {sensor_id} not found in registry")
                return

            # Create measurement
            measurement = PydanticMeasurement(
                process_id=sensor.process_id,
                sensor_id=sensor_id,
                rh=medicao,  # Using medicao as rh for now
                soc=100.0,  # Default SOC, can be added to payload later
                timestamp=datetime.datetime.now(SAO_PAULO_TZ),
            )

            saved = self.db_client.add_new_measurement(measurement)
            if saved:
                logger.info(
                    f"Measurement saved: sensor={sensor_id}, "
                    f"process={sensor.process_id}, rh={medicao}",
                )
            else:
                logger.error("Failed to save measurement to database")

        except (KeyError, ValueError, json.JSONDecodeError) as e:
            logger.error(f"Invalid measurement payload: {e}")

    def _handle_bind_request(self, payload: str) -> None:
        """
        Handle bind request messages.

        Expected payload: {"req_id": str, "nome": str}

        Args:
            payload (str): JSON payload string.
        """
        try:
            data = json.loads(payload)
            req_id = data["req_id"]
            nome = data["nome"]

            # Get the most recent active process (not ended)
            processes = self.db_client.get_all_processes()
            active_process = None
            for process in processes:
                if process.ended_at is None:
                    active_process = process
                    break

            if not active_process:
                logger.warning(
                    f"No active process found for bind request from {nome}",
                )
                # Send failure response
                response = json.dumps({
                    "req_id": req_id,
                    "id": "",
                    "status": "fail",
                })
                self.publisher.publish(self.TOPIC_BIND_RESPONSE, response)
                return

            # Create sensor in registry
            # Generate a simple sensor_id using timestamp + random component
            new_sensor_id = int(time.time() * 1000) % 1000000 + random.randint(  # noqa: S311
                1,
                1000,
            )

            created = self.db_client.register_new_sensor(
                new_sensor_id,
                active_process.id,
            )
            if created:
                logger.info(
                    f"Sensor {new_sensor_id} registered "
                    f"for process {active_process.id}",
                )

                # Send success response
                response = json.dumps({
                    "req_id": req_id,
                    "id": str(new_sensor_id),
                    "status": "ok",
                })
                self.publisher.publish(self.TOPIC_BIND_RESPONSE, response)
            else:
                logger.error("Failed to create sensor in database")
                # Send failure response
                response = json.dumps({
                    "req_id": req_id,
                    "id": "",
                    "status": "fail",
                })
                self.publisher.publish(self.TOPIC_BIND_RESPONSE, response)

        except (KeyError, ValueError, json.JSONDecodeError) as e:
            logger.error(f"Invalid bind request payload: {e}")

    def _handle_unbind(self, payload: str) -> None:  # noqa: PLR6301
        """
        Handle unbind messages.

        Expected payload: {"id": str}

        Note: Unbind does NOT delete data from database.
        All sensor data and measurements are preserved for historical records.

        Args:
            payload (str): JSON payload string.
        """
        try:
            data = json.loads(payload)
            sensor_id = int(data["id"])

            # Log unbind event but preserve all data in database
            logger.info(
                f"Sensor {sensor_id} unbound (data preserved for history)",
            )

        except (KeyError, ValueError, json.JSONDecodeError) as e:
            logger.error(f"Invalid unbind payload: {e}")
