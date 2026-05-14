#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <memory>
#include <vector>
#include "city/intersection.hpp"

namespace city {
    class City;
}

namespace traffic {
    class SemaphoreController;
    class Semaphore;
}

namespace traffic_simulation {
    class SimulationController;
}

namespace gui {

/**
 * @brief Simple vehicle representation for GUI rendering
 */
struct VehicleRenderInfo {
    int id;
    int x;
    int y;
    int destination_x;
    int destination_y;
    bool is_waiting;
    float angle;
    std::vector<city::Coordinate> remaining_path;
    
    VehicleRenderInfo(int vehicle_id, int start_x, int start_y, int dest_x, int dest_y, float ang = 0.0f)
        : id(vehicle_id), x(start_x), y(start_y),
          destination_x(dest_x), destination_y(dest_y),
          is_waiting(false), angle(ang) {}
};

/**
 * @brief Main window class for rendering the city traffic simulation using SFML
 */
class SimulationWindow {
public:
    SimulationWindow();
    ~SimulationWindow();

    // Non-copyable, non-movable
    SimulationWindow(const SimulationWindow&) = delete;
    SimulationWindow& operator=(const SimulationWindow&) = delete;
    SimulationWindow(SimulationWindow&&) = delete;
    SimulationWindow& operator=(SimulationWindow&&) = delete;

    /**
     * @brief Initialize the SFML render window
     * @param width Window width in pixels
     * @param height Window height in pixels
     * @param title Window title
     * @return true if initialization successful
     */
    bool initialize(int width, int height, const std::string& title);

    /**
     * @brief Set the city to render
     * @param city Pointer to the City object
     */
    void setCity(city::City* city);

    /**
     * @brief Set the list of vehicles to render
     * @param vehicles Pointer to vector of VehicleRenderInfo
     */
    void setVehicles(std::vector<VehicleRenderInfo>* vehicles);

    /**
     * @brief Set the semaphore controller for traffic light states
     * @param semaphores Pointer to SemaphoreController
     */
    void setSemaphoreController(traffic::SemaphoreController* semaphores);

    /**
     * @brief Set the cell size for rendering
     * @param cell_size Size of each grid cell in pixels
     */
    void setCellSize(int cell_size);

    /**
     * @brief Set the speed multiplier
     * @param multiplier Speed multiplier (1.0 = normal)
     */
    void setSpeedMultiplier(float multiplier);

    /**
     * @brief Check if the window is open
     * @return true if window is open
     */
    bool isOpen() const;

    /**
     * @brief Check if simulation is paused
     * @return true if paused
     */
    bool isPaused() const;

    /**
     * @brief Toggle pause state
     */
    void togglePause();

    /**
     * @brief Set pause state
     * @param paused New pause state
     */
    void setPaused(bool paused);

    /**
     * @brief Process window events (keyboard, mouse, close)
     */
    void handleEvents();

    /**
     * @brief Render the entire scene (city grid, vehicles, traffic lights)
     */
    void render();

    /**
     * @brief Run the main game loop with a simulation controller
     * @param controller Reference to simulation controller
     */
    void run(traffic_simulation::SimulationController& controller);

    /**
     * @brief Close the window
     */
    void close();

private:
    sf::RenderWindow window_;
    city::City* city_;
    std::vector<VehicleRenderInfo>* vehicles_;
    traffic::SemaphoreController* semaphores_;
    
    int cell_size_;
    float speed_multiplier_;
    bool is_paused_;
    bool window_closed_;
    
    // Colors for dark theme
    sf::Color background_color_;
    sf::Color grid_color_;
    sf::Color street_color_;
    sf::Color intersection_color_;
    
    // Traffic light colors
    sf::Color green_light_;
    sf::Color yellow_light_;
    sf::Color red_light_;
    
    // Cached shapes for efficiency
    sf::RectangleShape grid_background_;
    std::vector<sf::RectangleShape> intersection_shapes_;
    std::vector<sf::CircleShape> vehicle_shapes_;
    
    /**
     * @brief Draw the city grid background
     */
    void drawGrid();

    /**
     * @brief Draw all intersections with traffic light colors
     */
    void drawIntersections();

    /**
     * @brief Draw street connections between intersections
     */
    void drawStreets();

    /**
     * @brief Draw all vehicles as colored circles
     */
    void drawVehicles();

    /**
     * @brief Draw the control panel overlay
     */
    void drawControlPanel();

    /**
     * @brief Get traffic light color for an intersection
     * @param x Intersection x coordinate
     * @param y Intersection y coordinate
     * @return SFML Color for the current light state
     */
    sf::Color getTrafficLightColor(int x, int y);

    /**
     * @brief Update cached shapes for efficient rendering
     */
    void updateShapes();
};

} // namespace gui
