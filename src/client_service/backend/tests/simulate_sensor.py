#!/usr/bin/env python3
"""
Sensor Simulator for ESP32 Integration Testing

This script simulates the behavior of an ESP32 sensor with MQTT communication,
implementing the same state machine as the embedded code.
"""

import argparse
import json
import random
import time
import uuid
from enum import Enum

import paho.mqtt.client as mqtt


class Estado(Enum):
    """States of the sensor state machine."""

    BIND = "BIND"
    AGUARDE = "AGUARDE"
    MEDICAO = "MEDICAO"
    DEEP_SLEEP = "DEEP_SLEEP"
    CLEANUP = "CLEANUP"
    SHUTDOWN = "SHUTDOWN"


class SensorSimulator:
    """Simulates an ESP32 sensor with MQTT communication."""

    # MQTT Topics
    TOPIC_MEASUREMENT = "sensores/medicao"
    TOPIC_PROCESS = "sensores/processo"
    TOPIC_BIND_REQUEST = "sensores/bind/request"
    TOPIC_BIND_RESPONSE = "sensores/bind/response"
    TOPIC_UNBIND = "sensores/bind/unbind"
    TOPIC_STATUS = "sensores/status"

    def __init__(
        self,
        broker_host: str,
        broker_port: int,
        sensor_name: str,
        interval: int,
    ) -> None:
        """
        Initialize the sensor simulator.

        Args:
            broker_host: MQTT broker hostname.
            broker_port: MQTT broker port.
            sensor_name: Name of the sensor.
            interval: Measurement interval in seconds.
        """
        self.broker_host = broker_host
        self.broker_port = broker_port
        self.sensor_name = sensor_name
        self.interval = interval

        # State machine
        self.estado_atual = Estado.AGUARDE  # Começa aguardando, não em BIND
        self.processo_ativo = False
        self.processo_finalizado = False
        self.bind_ok = False
        self.req_id = ""
        self.sensor_id = ""

        # MQTT Client
        self.client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
        self.client.on_message = self._on_message
        self.client.on_connect = self._on_connect

        # Control
        self.running = True

    def _on_connect(
        self,
        client: mqtt.Client,  # noqa: ARG002
        userdata: None,  # noqa: ARG002
        flags: dict,  # noqa: ARG002
        rc: int,
        properties: None = None,  # noqa: ARG002
    ) -> None:
        """Callback when connected to MQTT broker."""
        if rc == 0:
            print(
                f"[✓] Connected to MQTT broker at {self.broker_host}:{self.broker_port}"
            )
            # Subscribe to process commands immediately
            self.client.subscribe(self.TOPIC_PROCESS)
            print(f"[✓] Subscribed to {self.TOPIC_PROCESS}")
        else:
            print(f"[✗] Failed to connect to MQTT broker: {rc}")

    def _on_message(
        self,
        client: mqtt.Client,  # noqa: ARG002
        userdata: None,  # noqa: ARG002
        msg: mqtt.MQTTMessage,
    ) -> None:
        """Callback when a message is received."""
        payload = msg.payload.decode("utf-8")

        if msg.topic == self.TOPIC_BIND_RESPONSE:
            self._handle_bind_response(payload)
        elif msg.topic == self.TOPIC_PROCESS:
            self._handle_process_command(payload)

    def _handle_bind_response(self, payload: str) -> None:
        """Handle bind response from server."""
        try:
            data = json.loads(payload)
            if data["req_id"] == self.req_id and data["status"] == "ok":
                self.sensor_id = data["id"]
                self.bind_ok = True
                print(f"[BIND] ✓ ID atribuído: {self.sensor_id}")
            elif data["status"] == "fail":
                print("[BIND] ✗ Falha ao obter ID")
                self.running = False
        except (json.JSONDecodeError, KeyError) as e:
            print(f"[ERROR] Invalid bind response: {e}")

    def _handle_process_command(self, payload: str) -> None:
        """Handle process command from server."""
        if payload == "iniciar":
            self.processo_ativo = True
            print("[PROCESSO] ⚡ Comando recebido: INICIAR")
        elif payload == "finalizar":
            self.processo_finalizado = True
            print("[PROCESSO] ⏹ Comando recebido: FINALIZAR")

    def _generate_uuid(self) -> str:
        """Generate a UUID for bind request."""
        return str(uuid.uuid4())

    def _handle_bind(self) -> None:
        """Handle BIND state."""
        print("[BIND] Solicitando ID ao servidor...")

        # Subscribe to bind response topic
        self.client.subscribe(self.TOPIC_BIND_RESPONSE)
        print(f"[BIND] ✓ Subscribed to {self.TOPIC_BIND_RESPONSE}")

        self.req_id = self._generate_uuid()

        request = {
            "req_id": self.req_id,
            "nome": self.sensor_name,
        }

        self.client.publish(self.TOPIC_BIND_REQUEST, json.dumps(request))
        print(f"[BIND] → Publicado em {self.TOPIC_BIND_REQUEST}")
        print(f"[BIND]   req_id: {self.req_id}")
        print(f"[BIND]   nome: {self.sensor_name}")

        # Wait for response
        timeout = 5
        start = time.time()
        while not self.bind_ok and time.time() - start < timeout:
            time.sleep(0.1)

        if self.bind_ok:
            self.estado_atual = Estado.MEDICAO
            print("[BIND] ✓ Transição para MEDICAO")
        else:
            print("[BIND] ✗ Timeout aguardando resposta")
            self.running = False

    def _handle_aguarde(self) -> None:
        """Handle AGUARDE state."""
        print("[AGUARDE] 💤 Aguardando comando 'iniciar'...")
        while not self.processo_ativo and self.running:
            time.sleep(0.5)

        if self.processo_ativo:
            self.estado_atual = Estado.BIND
            print("[AGUARDE] ✓ Comando 'iniciar' recebido, transição para BIND")

    def _realizar_medicao(self) -> None:
        """Perform a measurement and publish to MQTT."""
        # Simulate analog reading (random value between 0-100)
        medicao = round(random.uniform(0, 100), 2)

        message = {
            "medicao": medicao,
            "id": self.sensor_id,
        }

        self.client.publish(self.TOPIC_MEASUREMENT, json.dumps(message))
        print(
            f"[MEDICAO] 📊 Medição enviada: {medicao:.2f} (ID: {self.sensor_id})"
        )

    def _handle_medicao(self) -> None:
        """Handle MEDICAO state."""
        print("[MEDICAO] Realizando medição...")
        self._realizar_medicao()

        if self.processo_finalizado:
            self.estado_atual = Estado.CLEANUP
            print("[MEDICAO] ✓ Processo finalizado, indo para CLEANUP")
        else:
            self.estado_atual = Estado.DEEP_SLEEP
            print(f"[MEDICAO] ✓ Transição para DEEP_SLEEP ({self.interval}s)")

    def _handle_deep_sleep(self) -> None:
        """Handle DEEP_SLEEP state."""
        print(f"[SLEEP] 😴 Dormindo por {self.interval} segundos...")
        self.client.publish(self.TOPIC_STATUS, "sleeping")

        # Simulate deep sleep
        time.sleep(self.interval)

        print("[SLEEP] ⏰ Acordou do deep sleep")
        self.estado_atual = Estado.MEDICAO

    def _handle_cleanup(self) -> None:
        """Handle CLEANUP state."""
        print("[CLEANUP] 🧹 Limpando recursos...")
        self.client.publish(self.TOPIC_STATUS, "cleanup")

        # Send unbind message
        unbind = {"id": self.sensor_id}
        self.client.publish(self.TOPIC_UNBIND, json.dumps(unbind))
        print(f"[UNBIND] ✓ ID {self.sensor_id} liberado")

        self.estado_atual = Estado.SHUTDOWN

    def _handle_shutdown(self) -> None:
        """Handle SHUTDOWN state."""
        print("[SHUTDOWN] 🛑 Encerrando...")
        self.client.publish(self.TOPIC_STATUS, "shutdown")
        self.running = False

    def run(self) -> None:
        """Run the sensor simulator main loop."""
        print("=" * 60)
        print("🔧 ESP32 Sensor Simulator")
        print("=" * 60)
        print(f"Broker: {self.broker_host}:{self.broker_port}")
        print(f"Sensor: {self.sensor_name}")
        print(f"Interval: {self.interval}s")
        print("=" * 60)
        print(
            "\n[INFO] Fluxo: AGUARDE → (recebe 'iniciar') → BIND → MEDICAO → DEEP_SLEEP...\n"
        )

        # Connect to MQTT broker
        try:
            self.client.connect(self.broker_host, self.broker_port, 60)
            self.client.loop_start()
        except Exception as e:
            print(f"[✗] Failed to connect to MQTT broker: {e}")
            return

        # State machine loop
        try:
            while self.running:
                if self.estado_atual == Estado.AGUARDE:
                    self._handle_aguarde()
                elif self.estado_atual == Estado.BIND:
                    self._handle_bind()
                elif self.estado_atual == Estado.MEDICAO:
                    self._handle_medicao()
                elif self.estado_atual == Estado.DEEP_SLEEP:
                    self._handle_deep_sleep()
                elif self.estado_atual == Estado.CLEANUP:
                    self._handle_cleanup()
                elif self.estado_atual == Estado.SHUTDOWN:
                    self._handle_shutdown()

                time.sleep(0.1)

        except KeyboardInterrupt:
            print("\n[!] Interrompido pelo usuário")
        finally:
            print("[SHUTDOWN] Desconectando do broker...")
            self.client.loop_stop()
            self.client.disconnect()
            print("[✓] Simulador encerrado")


def main() -> None:
    """Main entry point."""
    parser = argparse.ArgumentParser(
        description="ESP32 Sensor Simulator for Integration Testing",
    )
    parser.add_argument(
        "--broker-host",
        type=str,
        default="localhost",
        help="MQTT broker hostname (default: localhost)",
    )
    parser.add_argument(
        "--broker-port",
        type=int,
        default=1883,
        help="MQTT broker port (default: 1883)",
    )
    parser.add_argument(
        "--sensor-name",
        type=str,
        default="esp32_sim",
        help="Sensor name (default: esp32_sim)",
    )
    parser.add_argument(
        "--interval",
        type=int,
        default=10,
        help="Measurement interval in seconds (default: 10)",
    )

    args = parser.parse_args()

    simulator = SensorSimulator(
        broker_host=args.broker_host,
        broker_port=args.broker_port,
        sensor_name=args.sensor_name,
        interval=args.interval,
    )

    simulator.run()


if __name__ == "__main__":
    main()
