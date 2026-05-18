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

class SimulationWindow {
public:
    SimulationWindow();
    ~SimulationWindow();

    SimulationWindow(const SimulationWindow&) = delete;
    SimulationWindow& operator=(const SimulationWindow&) = delete;
    SimulationWindow(SimulationWindow&&) = delete;
    SimulationWindow& operator=(SimulationWindow&&) = delete;

    bool initialize(int width, int height, const std::string& title);

    void setCity(city::City* city);
    void setVehicles(std::vector<VehicleRenderInfo>* vehicles);
    void setSemaphoreController(traffic::SemaphoreController* semaphores);
    void setCellSize(int cell_size);
    void setSpeedMultiplier(float multiplier);

    bool isOpen() const;
    bool isPaused() const;

    void togglePause();
    void setPaused(bool paused);

    void handleEvents();
    void render();

    /**
     * @brief Main render loop. Stores controller reference for button actions.
     */
    void run(traffic_simulation::SimulationController& controller);

    void close();

private:
    // ---- Height of the top toolbar in pixels ----
    static constexpr int TOOLBAR_HEIGHT = 55;

    sf::RenderWindow window_;
    city::City* city_;
    std::vector<VehicleRenderInfo>* vehicles_;
    traffic::SemaphoreController* semaphores_;

    int cell_size_;
    float speed_multiplier_;
    bool is_paused_;
    bool window_closed_;

    // Font for toolbar text and button labels
    sf::Font font_;
    bool font_loaded_;

    // Pointer to controller so buttons can call pause/resume/stepOnce
    traffic_simulation::SimulationController* controller_;

    // Bounding rectangles for the 3 toolbar buttons (updated each frame in drawToolbar)
    sf::FloatRect pause_btn_bounds_;
    sf::FloatRect continue_btn_bounds_;
    sf::FloatRect step_btn_bounds_;

    // ---- Light minimalist color palette ----
    sf::Color background_color_;   // warm cream city-block fill
    sf::Color grid_color_;         // subtle grid line
    sf::Color street_color_;       // road surface
    sf::Color intersection_color_; // neutral intersection (no light)
    sf::Color green_light_;
    sf::Color yellow_light_;
    sf::Color red_light_;

    // ---- Draw helpers ----
    void drawGrid();
    void drawStreets();
    void drawIntersections();
    void drawStreetDirections();
    void drawStreetNames();
    void drawVehicles();
    void drawToolbar();

    sf::Color getTrafficLightColor(int x, int y);
};

} // namespace gui
