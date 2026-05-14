/**
 * @file main.cpp
 * @brief Main entry point for the city traffic simulation
 * @author Traffic Simulation Team
 * @version 1.0
 */

#include "common/simulation_controller.hpp"
#include "common/logger.hpp"
#include "common/constants.hpp"
#include "gui/simulation_window.hpp"
using namespace traffic_simulation;

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <chrono>

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace traffic_simulation;
/**
 * @brief Print usage information
 */
void printUsage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [options]\n"
              << "Options:\n"
              << "  --no-gui           Run in headless mode without GUI\n"
              << "  --vehicles N       Number of vehicles (default: " << constants::DEFAULT_VEHICLE_COUNT << ")\n"
              << "  --grid N           Grid size NxN (default: " << constants::DEFAULT_GRID_SIZE << ")\n"
              << "  --green N          Green light duration in seconds (default: " << constants::DEFAULT_GREEN_DURATION << ")\n"
              << "  --yellow N         Yellow light duration in seconds (default: " << constants::DEFAULT_YELLOW_DURATION << ")\n"
              << "  --red N            Red light duration in seconds (default: " << constants::DEFAULT_RED_DURATION << ")\n"
              << "  --help             Show this help message\n";
}

/**
 * @brief Parse command line arguments into SimulationConfig
 */
traffic_simulation::SimulationConfig parseArguments(int argc, char* argv[]) {
    traffic_simulation::SimulationConfig config;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--help") {
            printUsage(argv[0]);
            std::exit(0);
        } else if (arg == "--no-gui") {
            // This is handled separately in main
            config.grid_size = -1; // Signal to use headless mode
        } else if (arg == "--vehicles" && i + 1 < argc) {
            int vehicles = std::atoi(argv[++i]);
            if (vehicles < constants::MIN_VEHICLE_COUNT) {
                vehicles = constants::MIN_VEHICLE_COUNT;
            } else if (vehicles > constants::MAX_VEHICLE_COUNT) {
                vehicles = constants::MAX_VEHICLE_COUNT;
            }
            config.vehicle_count = vehicles;
        } else if (arg == "--grid" && i + 1 < argc) {
            int grid = std::atoi(argv[++i]);
            if (grid < constants::MIN_GRID_SIZE) {
                grid = constants::MIN_GRID_SIZE;
            } else if (grid > constants::MAX_GRID_SIZE) {
                grid = constants::MAX_GRID_SIZE;
            }
            config.grid_size = grid;
        } else if (arg == "--green" && i + 1 < argc) {
            config.green_duration = std::atoi(argv[++i]);
            if (config.green_duration < 1) {
                config.green_duration = 1;
            }
        } else if (arg == "--yellow" && i + 1 < argc) {
            config.yellow_duration = std::atoi(argv[++i]);
            if (config.yellow_duration < 1) {
                config.yellow_duration = 1;
            }
        } else if (arg == "--red" && i + 1 < argc) {
            config.red_duration = std::atoi(argv[++i]);
            if (config.red_duration < 1) {
                config.red_duration = 1;
            }
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            printUsage(argv[0]);
            std::exit(1);
        }
    }

    return config;
}

/**
 * @brief Run simulation in headless mode (no GUI)
 */
int runHeadless(traffic_simulation::SimulationController& controller) {
    std::cout << "Running in headless mode...\n";

    controller.run();

    // Wait for all vehicles to complete
    while (controller.isRunning() && controller.getVehicleManager().getActiveVehicleCount() != 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Give some time for all vehicles to complete
    std::cout << "Waiting for vehicles to complete...\n";
    controller.getVehicleManager().waitForCompletion();

    // Output metrics
    std::cout << "\n========== SIMULATION RESULTS ==========\n";
    controller.getMetrics().exportToTable("");

    return 0;
}

/**
 * @brief Run simulation with GUI
 */
int runWithGUI(traffic_simulation::SimulationController& controller, 
               traffic_simulation::SimulationConfig& /*config*/) {
    std::cout << "Running with GUI...\n";

    // Initialize GUI window
    gui::SimulationWindow window;
    if (!window.initialize(800, 600, "City Traffic Simulation")) {
        std::cerr << "Failed to initialize SFML window\n";
        return 1;
    }

    window.setCity(&controller.getCity());
    window.setSemaphoreController(&controller.getSemaphoreController());
    window.setCellSize(40);

    // Start simulation
    controller.run();

    window.run(controller);

    // When window is closed, simulation will stop
    return 0;
}

int main(int argc, char* argv[]) {
    // Parse arguments
    traffic_simulation::SimulationConfig config = parseArguments(argc, argv);

    // Check for headless mode
    bool headless_mode = false;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--no-gui") == 0) {
            headless_mode = true;
            break;
        }
    }

    // Apply defaults if not set via command line
    if (config.grid_size <= 0) {
        config.grid_size = constants::DEFAULT_GRID_SIZE;
    }
    if (config.vehicle_count <= 0) {
        config.vehicle_count = constants::DEFAULT_VEHICLE_COUNT;
    }

    // Print configuration
    std::cout << "========== CITY TRAFFIC SIMULATION ==========\n";
    std::cout << "Grid size: " << config.grid_size << "x" << config.grid_size << "\n";
    std::cout << "Vehicles: " << config.vehicle_count << "\n";
    std::cout << "Timing: green=" << config.green_duration 
              << "s, yellow=" << config.yellow_duration 
              << "s, red=" << config.red_duration << "s\n";
    std::cout << "Mode: " << (headless_mode ? "HEADLESS" : "GUI") << "\n";
    std::cout << "============================================\n\n";

#ifdef _OPENMP
    std::cout << "OpenMP enabled. Max threads: " << omp_get_max_threads() << "\n";
#endif

    // Create and initialize simulation controller
    traffic_simulation::SimulationController controller;

    try {
        controller.initialize(config);
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize simulation: " << e.what() << "\n";
        return 1;
    }

    int exit_code;
    if (headless_mode) {
        exit_code = runHeadless(controller);
    } else {
        exit_code = runWithGUI(controller, config);
    }

    std::cout << "Simulation terminated with exit code: " << exit_code << "\n";
    return exit_code;
}
