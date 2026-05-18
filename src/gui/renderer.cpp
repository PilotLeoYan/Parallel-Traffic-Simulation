#include "gui/renderer.hpp"
#include <cmath>

namespace gui {

void Renderer::drawGrid(sf::RenderWindow& window,
                         int grid_size,
                         int cell_size,
                         const sf::Color& background_color,
                         const sf::Color& grid_line_color) {
    int total_size = grid_size * cell_size;

    sf::RectangleShape background(sf::Vector2f(total_size, total_size));
    background.setFillColor(background_color);
    window.draw(background);

    for (int i = 0; i <= grid_size; ++i) {
        sf::RectangleShape v_line(sf::Vector2f(1, total_size));
        v_line.setPosition(i * cell_size, 0);
        v_line.setFillColor(grid_line_color);
        window.draw(v_line);

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
    sf::RectangleShape intersection(sf::Vector2f(size, size));
    intersection.setPosition(x, y);
    intersection.setFillColor(base_color);
    window.draw(intersection);

    sf::CircleShape indicator(size / 4.0f);
    indicator.setPosition(x + size / 2.0f - size / 4.0f,
                          y + size / 2.0f - size / 4.0f);

    sf::Color light_color;
    switch (light_state) {
        case 0: light_color = sf::Color(34,  197,  94); break;
        case 1: light_color = sf::Color(234, 179,   8); break;
        case 2: light_color = sf::Color(239,  68,  68); break;
        default: light_color = sf::Color(160, 160, 160);
    }

    indicator.setFillColor(light_color);
    window.draw(indicator);

    sf::CircleShape glow(size / 3.0f);
    glow.setPosition(x + size / 2.0f - size / 3.0f,
                     y + size / 2.0f - size / 3.0f);
    glow.setFillColor(sf::Color(light_color.r, light_color.g, light_color.b, 40));
    window.draw(glow);
}

// ----------------------------------------------------------------
// drawVehicle -- tuned for visibility on light backgrounds
// ----------------------------------------------------------------

void Renderer::drawVehicle(sf::RenderWindow& window,
                            float x, float y, float radius,
                            int vehicle_id,
                            bool is_waiting,
                            float angle_degrees) {
    sf::Color bodyColor = getVehicleColor(vehicle_id);

    float carWidth  = radius * 2.1f;
    float carHeight = radius * 1.15f;

    // On light backgrounds a reduced alpha is still readable; just lower it less
    sf::Uint8 alpha = is_waiting ? 200 : 255;
    bodyColor.a = alpha;

    sf::Transform transform;
    transform.translate(x, y);
    transform.rotate(angle_degrees);

    // 1. Car body with thicker outline for contrast on light road
    sf::RectangleShape carBody(sf::Vector2f(carWidth, carHeight));
    carBody.setOrigin(carWidth / 2.0f, carHeight / 2.0f);
    carBody.setFillColor(bodyColor);
    carBody.setOutlineColor(sf::Color(30, 28, 25, alpha));
    carBody.setOutlineThickness(1.8f);  // increased from 1.0f
    window.draw(carBody, transform);

    // 2. Roof / windshield
    float roofWidth  = carWidth * 0.48f;
    float roofHeight = carHeight * 0.76f;
    sf::RectangleShape roof(sf::Vector2f(roofWidth, roofHeight));
    roof.setOrigin(roofWidth / 2.0f, roofHeight / 2.0f);
    roof.setPosition(-carWidth * 0.08f, 0.0f);
    roof.setFillColor(sf::Color(25, 23, 20, alpha));
    window.draw(roof, transform);

    // 3. Brake lights (red strip on rear) when waiting / blocked
    if (is_waiting) {
        sf::RectangleShape tl(sf::Vector2f(carWidth * 0.09f, carHeight * 0.78f));
        tl.setOrigin(carWidth * 0.045f, carHeight * 0.39f);
        tl.setPosition(-carWidth * 0.5f, 0.0f);
        tl.setFillColor(sf::Color(255, 45, 45));

        sf::RectangleShape glow(sf::Vector2f(carWidth * 0.22f, carHeight * 1.0f));
        glow.setOrigin(carWidth * 0.11f, carHeight * 0.5f);
        glow.setPosition(-carWidth * 0.5f, 0.0f);
        glow.setFillColor(sf::Color(255, 45, 45, 70));

        window.draw(glow, transform);
        window.draw(tl,   transform);
    }
}

void Renderer::drawStreet(sf::RenderWindow& window,
                           float start_x, float start_y,
                           float end_x, float end_y,
                           float street_width,
                           const sf::Color& color) {
    float dx     = end_x - start_x;
    float dy     = end_y - start_y;
    float length = std::sqrt(dx * dx + dy * dy);
    if (length < 1.0f) return;

    float angle = std::atan2(dy, dx) * 180.0f / 3.14159265f;

    sf::RectangleShape street(sf::Vector2f(length, street_width));
    street.setPosition(start_x, start_y - street_width / 2.0f);
    street.setFillColor(color);
    street.setRotation(angle);

    sf::Transform transform;
    transform.rotate(angle, start_x, start_y);
    window.draw(street, transform);
}

// ----------------------------------------------------------------
// getVehicleColor -- HSV distribution; vivid on both light & dark
// ----------------------------------------------------------------

sf::Color Renderer::getVehicleColor(int vehicle_id) {
    int   hue        = (vehicle_id * 37) % 360;
    float saturation = 0.72f + (vehicle_id % 3) * 0.09f;
    float value      = 0.88f;

    int   hi = (hue / 60) % 6;
    float f  = (hue / 60.0f) - hi;
    float l  = value * (1.0f - saturation);
    float m  = value * (1.0f - f * saturation);
    float n  = value * (1.0f - (1.0f - f) * saturation);

    float r, g, b;
    switch (hi) {
        case 0: r = value; g = n;     b = l;     break;
        case 1: r = m;     g = value; b = l;     break;
        case 2: r = l;     g = value; b = n;     break;
        case 3: r = l;     g = m;     b = value; break;
        case 4: r = n;     g = l;     b = value; break;
        case 5: r = value; g = l;     b = m;     break;
        default: r = value; g = l;    b = n;
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
        case 0: light.setFillColor(sf::Color( 34, 197,  94)); break;
        case 1: light.setFillColor(sf::Color(234, 179,   8)); break;
        case 2: light.setFillColor(sf::Color(239,  68,  68)); break;
        default: light.setFillColor(sf::Color(160, 160, 160));
    }
    window.draw(light);
}

sf::RectangleShape Renderer::makeRoundedRect(float x, float y, float width, float height,
                                              const sf::Color& color, float /*corner_radius*/) {
    sf::RectangleShape rect(sf::Vector2f(width, height));
    rect.setPosition(x, y);
    rect.setFillColor(color);
    return rect;
}

void Renderer::drawArrow(sf::RenderWindow& window,
                         float x, float y,
                         float angle_degrees,
                         float size,
                         const sf::Color& color) {
    sf::ConvexShape arrow(3);
    arrow.setPoint(0, sf::Vector2f( size,          0.f));
    arrow.setPoint(1, sf::Vector2f(-size * 0.5f,   size * 0.5f));
    arrow.setPoint(2, sf::Vector2f(-size * 0.5f,  -size * 0.5f));
    arrow.setOrigin(0.f, 0.f);
    arrow.setPosition(x, y);
    arrow.setRotation(angle_degrees);
    arrow.setFillColor(color);
    window.draw(arrow);
}

} // namespace gui
