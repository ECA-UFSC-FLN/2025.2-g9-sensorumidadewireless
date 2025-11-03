"""
MQTT interfaces for publisher and consumer.

This module defines abstract base classes for MQTT operations,
allowing for easy implementation swapping without code changes.
"""

from abc import ABC, abstractmethod


class IMQTTPublisher(ABC):
    """Interface for MQTT publisher implementations."""

    @abstractmethod
    def connect(self) -> bool:
        """
        Connect to the MQTT broker.

        Returns:
            bool: True if connection successful, False otherwise.
        """

    @abstractmethod
    def disconnect(self) -> None:
        """Disconnect from the MQTT broker."""

    @abstractmethod
    def publish(self, topic: str, payload: str) -> bool:
        """
        Publish a message to a topic.

        Args:
            topic (str): The MQTT topic to publish to.
            payload (str): The message payload.

        Returns:
            bool: True if publish successful, False otherwise.
        """


class IMQTTConsumer(ABC):
    """Interface for MQTT consumer implementations."""

    @abstractmethod
    def connect(self) -> bool:
        """
        Connect to the MQTT broker.

        Returns:
            bool: True if connection successful, False otherwise.
        """

    @abstractmethod
    def disconnect(self) -> None:
        """Disconnect from the MQTT broker."""

    @abstractmethod
    def subscribe(self, topic: str) -> bool:
        """
        Subscribe to a topic.

        Args:
            topic (str): The MQTT topic to subscribe to.

        Returns:
            bool: True if subscription successful, False otherwise.
        """

    @abstractmethod
    def start(self) -> None:
        """Start the consumer loop in a separate thread."""

    @abstractmethod
    def stop(self) -> None:
        """Stop the consumer loop and cleanup resources."""
