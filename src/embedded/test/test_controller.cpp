#include "../include/MainController.h"
#include "mock_implementations.h"
#include <iostream>
#include <thread>
#include <chrono>

// Function to simulate process start/stop commands
// Counter for measurements
static int measurementCount = 0;

// Callback to count measurements
void countMeasurements(const char* topic, const char* payload) {
    if (strcmp(topic, "sensores/medicao") == 0) {
        try {
            auto j = nlohmann::json::parse(payload);
            // Valida se é uma medição válida verificando os campos necessários
            if (j.contains("medicao") && j.contains("id")) {
                measurementCount++;
                std::cout << "Valid measurement received. Count: " << measurementCount 
                          << " Value: " << j["medicao"].get<float>() 
                          << " ID: " << j["id"].get<std::string>() << std::endl;
            } else {
                std::cerr << "Invalid measurement format: " << payload << std::endl;
            }
        } catch (const nlohmann::json::exception& e) {
            std::cerr << "Failed to parse measurement: " << e.what() << std::endl;
        }
    }
}

void simulateProcessCommands(MainController& controller, MockMQTT& mqtt) {
    // Set up measurement counting
    mqtt.setPublishInterceptor(countMeasurements);
    
    // Wait a bit for everything to initialize
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Simulate process start command
    mqtt.simulateMessage("sensores/processo", "iniciar");
    
    // Wait until we get exactly 3 measurements
    while (measurementCount < 3) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "Got 3 measurements, sending finalizar..." << std::endl;
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

    // Run the main loop until shutdown or timeout
    auto startTime = std::chrono::steady_clock::now();
    const auto timeout = std::chrono::seconds(60);
    bool testCompleted = false;
    
    std::cout << "Starting main loop..." << std::endl;
    
    while (std::chrono::steady_clock::now() - startTime < timeout) {
        controller.loop();
        
        // Verifica se chegou ao estado SHUTDOWN após as medições
        if (measurementCount >= 3 && controller.getState() == MainController::SHUTDOWN) {
            std::cout << "Test completed successfully - controller reached SHUTDOWN state" << std::endl;
            testCompleted = true;
            break;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Print progress every 5 seconds
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - startTime);
            
        if (elapsed.count() % 5 == 0) {
            std::cout << "Test running for " << elapsed.count() << "s, measurement count: " 
                      << measurementCount << ", state: " << controller.getState() << std::endl;
        }
    }

    if (!testCompleted) {
        std::cerr << "Test failed - timeout reached after " << timeout.count() 
                  << "s without reaching SHUTDOWN state" << std::endl;
        return 1;
    }

    // Wait for the simulation thread to finish
    processThread.join();

    return 0;
}