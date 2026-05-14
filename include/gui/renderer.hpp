#pragma once

#include <SFML/Graphics.hpp>

namespace gui {

/**
 * @brief Utility class for drawing primitives in the simulation
 */
class Renderer {
public:
    /**
     * @brief Draw the city grid background
     * @param window Render target
     * @param grid_size Size of the grid (number of cells per dimension)
     * @param cell_size Size of each cell in pixels
     * @param background_color Background color
     * @param grid_line_color Grid line color
     */
    static void drawGrid(sf::RenderWindow& window,
                         int grid_size,
                         int cell_size,
                         const sf::Color& background_color = sf::Color(30, 30, 30),
                         const sf::Color& grid_line_color = sf::Color(50, 50, 50));

    /**
     * @brief Draw an intersection with traffic light indicator
     * @param window Render target
     * @param x X position in pixels
     * @param y Y position in pixels
     * @param size Size of the intersection square
     * @param light_state Current traffic light state (0=green, 1=yellow, 2=red)
     * @param base_color Base color for the intersection
     */
    static void drawIntersection(sf::RenderWindow& window,
                                 float x, float y, float size,
                                 int light_state,
                                 const sf::Color& base_color = sf::Color(80, 80, 80));

    /**
     * @brief Draw a vehicle as a colored circle
     * @param window Render target
     * @param x X position in pixels (center)
     * @param y Y position in pixels (center)
     * @param radius Radius of the vehicle circle
     * @param vehicle_id ID used to generate consistent color per vehicle
     * @param is_waiting If true, draw with reduced opacity
     */
    static void drawVehicle(sf::RenderWindow& window,
                        float x, float y, float radius,
                        int vehicle_id,
                        bool is_waiting,
                        float angle_degrees = 0.0f);

    /**
     * @brief Draw a street segment between two intersections
     * @param window Render target
     * @param start_x Start X position in pixels
     * @param start_y Start Y position in pixels
     * @param end_x End X position in pixels
     * @param end_y End Y position in pixels
     * @param street_width Width of the street line
     * @param color Color of the street
     */
    static void drawStreet(sf::RenderWindow& window,
                           float start_x, float start_y,
                           float end_x, float end_y,
                           float street_width = 4.0f,
                           const sf::Color& color = sf::Color(100, 100, 100));

    /**
     * @brief Generate a consistent color from a vehicle ID
     * @param vehicle_id Vehicle identifier
     * @return SFML Color for the vehicle
     */
    static sf::Color getVehicleColor(int vehicle_id);

    /**
     * @brief Draw a traffic light indicator circle
     * @param window Render target
     * @param x Center X position
     * @param y Center Y position
     * @param radius Circle radius
     * @param state Traffic light state (0=green, 1=yellow, 2=red)
     */
    static void drawTrafficLight(sf::RenderWindow& window,
                                 float x, float y, float radius,
                                 int state);

    /**
     * @brief Draw a rounded rectangle shape (utility for UI elements)
     * @param x Top-left X position
     * @param y Top-left Y position
     * @param width Rectangle width
     * @param height Rectangle height
     * @param color Fill color
     * @param corner_radius Corner radius for rounding
     * @return sf::RectangleShape configured
     */
    static sf::RectangleShape makeRoundedRect(float x, float y, float width, float height,
                                               const sf::Color& color, float corner_radius = 0.0f);
};

} // namespace gui
