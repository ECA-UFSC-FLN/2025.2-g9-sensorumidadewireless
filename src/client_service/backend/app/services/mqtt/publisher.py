"""
MQTT Publisher implementation using paho-mqtt.
"""

import paho.mqtt.client as mqtt

from app.services.mqtt.interfaces import IMQTTPublisher
from app.utils.logger import logger


class PahoMQTTPublisher(IMQTTPublisher):
    """MQTT Publisher implementation using paho-mqtt library."""

    def __init__(self, broker_host: str, broker_port: int) -> None:
        """
        Initialize the MQTT publisher.

        Args:
            broker_host (str): MQTT broker hostname.
            broker_port (int): MQTT broker port.
        """
        self.broker_host = broker_host
        self.broker_port = broker_port
        self.client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
        self._connected = False

    def connect(self) -> bool:
        """
        Connect to the MQTT broker.

        Returns:
            bool: True if connection successful, False otherwise.
        """
        try:
            self.client.connect(self.broker_host, self.broker_port, 60)
            self.client.loop_start()
            self._connected = True
            logger.info(
                f"MQTT Publisher connected to "
                f"{self.broker_host}:{self.broker_port}",
            )
            return True
        except Exception as e:
            logger.error(f"Failed to connect MQTT Publisher: {e}")
            self._connected = False
            return False

    def disconnect(self) -> None:
        """Disconnect from the MQTT broker."""
        if self._connected:
            self.client.loop_stop()
            self.client.disconnect()
            self._connected = False
            logger.info("MQTT Publisher disconnected")

    def publish(self, topic: str, payload: str, retained: bool = False) -> bool:
        """
        Publish a message to a topic.

        Args:
            topic (str): The MQTT topic to publish to.
            payload (str): The message payload.
            retained (bool): Whether to retain the message.
        Returns:
            bool: True if publish successful, False otherwise.
        """
        if not self._connected:
            logger.error("Cannot publish: MQTT Publisher not connected")
            return False

        try:
            result = self.client.publish(topic, payload, retain=retained)
            if result.rc == mqtt.MQTT_ERR_SUCCESS:
                logger.info(f"Published to {topic}: {payload}")
                return True
            logger.error(f"Failed to publish to {topic}: {result.rc}")
            return False
        except Exception as e:
            logger.error(f"Error publishing to {topic}: {e}")
            return False

    def publish_process_command(
        self, command: str, retained: bool = False
    ) -> bool:
        """
        Publish a process command to the sensors.

        Args:
            command (str): Command to send ("iniciar" or "finalizar").
            retained (bool): Whether to retain the message.
        Returns:
            bool: True if publish successful, False otherwise.
        """
        return self.publish("sensores/processo", command, retained=retained)
