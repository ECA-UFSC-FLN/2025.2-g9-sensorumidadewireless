#include "../include/MainController.h"
#include "mock_implementations.h"
#include <iostream>
#include <thread>
#include <chrono>

// Function to simulate process start/stop commands
void simulateProcessCommands(MainController& controller, MockMQTT& mqtt) {
    // Wait a bit for everything to initialize
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Simulate process start command
    mqtt.simulateMessage("sensores/processo", "iniciar");
    
    // Wait for some measurements
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    // Simulate process stop command
    mqtt.simulateMessage("sensores/processo", "finalizar");
}

int main() {
    // Create mock implementations
    MockHardware hardware;
    MockMQTT mqtt;
    MockJSON json;
    MockLogger logger;

    // Create controller with mock implementations
    MainController controller(hardware, mqtt, json, logger);

    // Create a thread to simulate incoming MQTT commands
    std::thread processThread(simulateProcessCommands, std::ref(controller), std::ref(mqtt));

    // Run the main loop for a while
    auto startTime = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - startTime < std::chrono::seconds(10)) {
        controller.loop();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Wait for the simulation thread to finish
    processThread.join();

    return 0;
}