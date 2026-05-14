#include "gui/renderer.hpp"
#include <cmath>

namespace gui {

void Renderer::drawGrid(sf::RenderWindow& window,
                         int grid_size,
                         int cell_size,
                         const sf::Color& background_color,
                         const sf::Color& grid_line_color) {
    int total_size = grid_size * cell_size;
    
    // Draw background
    sf::RectangleShape background(sf::Vector2f(total_size, total_size));
    background.setFillColor(background_color);
    window.draw(background);
    
    // Draw grid lines
    for (int i = 0; i <= grid_size; ++i) {
        // Vertical lines
        sf::RectangleShape v_line(sf::Vector2f(1, total_size));
        v_line.setPosition(i * cell_size, 0);
        v_line.setFillColor(grid_line_color);
        window.draw(v_line);
        
        // Horizontal lines
        sf::RectangleShape h_line(sf::Vector2f(total_size, 1));
        h_line.setPosition(0, i * cell_size);
        h_line.setFillColor(grid_line_color);
        window.draw(h_line);
    }
}

void Renderer::drawIntersection(sf::RenderWindow& window,
                                float x, float y, float size,
                                int light_state,
                                const sf::Color& base_color) {
    // Draw intersection base
    sf::RectangleShape intersection(sf::Vector2f(size, size));
    intersection.setPosition(x, y);
    intersection.setFillColor(base_color);
    window.draw(intersection);
    
    // Draw traffic light indicator
    sf::CircleShape indicator(size / 4.0f);
    indicator.setPosition(x + size / 2.0f - size / 4.0f,
                         y + size / 2.0f - size / 4.0f);
    
    sf::Color light_color;
    switch (light_state) {
        case 0: light_color = sf::Color(0, 255, 100); break;   // Green
        case 1: light_color = sf::Color(255, 220, 0); break;   // Yellow
        case 2: light_color = sf::Color(255, 50, 50); break;    // Red
        default: light_color = sf::Color(100, 100, 100);
    }
    
    indicator.setFillColor(light_color);
    window.draw(indicator);
    
    // Add subtle glow effect
    sf::CircleShape glow(size / 3.0f);
    glow.setPosition(x + size / 2.0f - size / 3.0f,
                    y + size / 2.0f - size / 3.0f);
    glow.setFillColor(sf::Color(light_color.r, light_color.g, light_color.b, 40));
    window.draw(glow);
}

void Renderer::drawVehicle(sf::RenderWindow& window,
                            float x, float y, float radius,
                            int vehicle_id,
                            bool is_waiting,
                            float angle_degrees) {
    sf::Color bodyColor = getVehicleColor(vehicle_id);
    
    // Dimensiones del auto basadas en el radio de la celda
    float carWidth = radius * 2.2f;  // Largo del auto
    float carHeight = radius * 1.2f; // Ancho del auto
    
    // Si está esperando, reducimos un poco la opacidad
    sf::Uint8 alpha = is_waiting ? 180 : 255;
    bodyColor.a = alpha;

    // Transformación principal para rotar todo el auto desde su centro
    sf::Transform transform;
    transform.translate(x, y);
    transform.rotate(angle_degrees);

    // 1. Chasis principal
    sf::RectangleShape carBody(sf::Vector2f(carWidth, carHeight));
    carBody.setOrigin(carWidth / 2.0f, carHeight / 2.0f);
    carBody.setFillColor(bodyColor);
    carBody.setOutlineColor(sf::Color(30, 30, 30, alpha));
    carBody.setOutlineThickness(1.0f);
    window.draw(carBody, transform);

    // 2. Techo / Parabrisas (Un rectángulo más oscuro en el centro)
    float roofWidth = carWidth * 0.5f;
    float roofHeight = carHeight * 0.8f;
    sf::RectangleShape roof(sf::Vector2f(roofWidth, roofHeight));
    roof.setOrigin(roofWidth / 2.0f, roofHeight / 2.0f);
    // Lo movemos un poco hacia atrás para que el "cofre" (frente) sea más largo
    roof.setPosition(-carWidth * 0.1f, 0.0f); 
    roof.setFillColor(sf::Color(20, 20, 25, alpha));
    window.draw(roof, transform);

    // 3. Luces de freno (Solo se encienden si está esperando/bloqueado)
    if (is_waiting) {
        sf::RectangleShape tailLights(sf::Vector2f(carWidth * 0.1f, carHeight * 0.8f));
        tailLights.setOrigin(carWidth * 0.05f, carHeight * 0.4f);
        // Posicionadas en la parte trasera del auto
        tailLights.setPosition(-carWidth / 2.0f, 0.0f);
        tailLights.setFillColor(sf::Color(255, 40, 40));
        
        // Efecto de brillo (Glow)
        sf::RectangleShape tailLightsGlow(sf::Vector2f(carWidth * 0.2f, carHeight * 1.0f));
        tailLightsGlow.setOrigin(carWidth * 0.1f, carHeight * 0.5f);
        tailLightsGlow.setPosition(-carWidth / 2.0f, 0.0f);
        tailLightsGlow.setFillColor(sf::Color(255, 40, 40, 80));

        window.draw(tailLightsGlow, transform);
        window.draw(tailLights, transform);
    }
}

void Renderer::drawStreet(sf::RenderWindow& window,
                           float start_x, float start_y,
                           float end_x, float end_y,
                           float street_width,
                           const sf::Color& color) {
    // Calculate direction and length
    float dx = end_x - start_x;
    float dy = end_y - start_y;
    float length = std::sqrt(dx * dx + dy * dy);
    
    if (length < 1.0f) return;
    
    // Calculate angle
    float angle = std::atan2(dy, dx) * 180.0f / 3.14159265f;
    
    // Draw street line
    sf::RectangleShape street(sf::Vector2f(length, street_width));
    street.setPosition(start_x, start_y - street_width / 2.0f);
    street.setFillColor(color);
    street.setRotation(angle);
    
    // Adjust position to center properly after rotation
    sf::Transform transform;
    transform.rotate(angle, start_x, start_y);
    
    window.draw(street, transform);
}

sf::Color Renderer::getVehicleColor(int vehicle_id) {
    // Generate a consistent color based on vehicle ID
    // Use HSV to RGB conversion for better color distribution
    int hue = (vehicle_id * 37) % 360;  // Golden ratio for good distribution
    float saturation = 0.7f + (vehicle_id % 3) * 0.1f;
    float value = 0.9f;
    
    // HSV to RGB conversion
    int hi = (hue / 60) % 6;
    float f = (hue / 60.0f) - hi;
    float l = value * (1.0f - saturation);
    float m = value * (1.0f - f * saturation);
    float n = value * (1.0f - (1.0f - f) * saturation);
    
    float r, g, b;
    switch (hi) {
        case 0: r = value; g = n; b = l; break;
        case 1: r = m; g = value; b = l; break;
        case 2: r = l; g = value; b = n; break;
        case 3: r = l; g = m; b = value; break;
        case 4: r = n; g = l; b = value; break;
        case 5: r = value; g = l; b = m; break;
        default: r = value; g = l; b = n;
    }
    
    return sf::Color(static_cast<sf::Uint8>(r * 255),
                    static_cast<sf::Uint8>(g * 255),
                    static_cast<sf::Uint8>(b * 255));
}

void Renderer::drawTrafficLight(sf::RenderWindow& window,
                                 float x, float y, float radius,
                                 int state) {
    sf::CircleShape light(radius);
    light.setPosition(x - radius, y - radius);
    
    switch (state) {
        case 0: light.setFillColor(sf::Color(0, 255, 100)); break;   // Green
        case 1: light.setFillColor(sf::Color(255, 220, 0)); break;   // Yellow
        case 2: light.setFillColor(sf::Color(255, 50, 50)); break;   // Red
        default: light.setFillColor(sf::Color(100, 100, 100));
    }
    
    window.draw(light);
}

sf::RectangleShape Renderer::makeRoundedRect(float x, float y, float width, float height,
                                              const sf::Color& color, float corner_radius) {
    sf::RectangleShape rect(sf::Vector2f(width, height));
    rect.setPosition(x, y);
    rect.setFillColor(color);
    return rect;
}

} // namespace gui
