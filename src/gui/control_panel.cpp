#include "gui/control_panel.hpp"
#include <common/types.hpp>

namespace gui {

ControlPanel::ControlPanel()
    : vehicle_count_value_(50),
      green_duration_value_(5),
      yellow_duration_value_(2),
      red_duration_value_(6),
      speed_multiplier_value_(1.0f),
      is_paused_(false),
      reset_requested_(false),
      panel_x_(20),
      panel_y_(20),
      panel_width_(200),
      panel_height_(400) {
    
    initializeText();
}

void ControlPanel::initializeText() {
    // cargar fuentes de Windows y luego las de Linux
    if (!font_.loadFromFile("C:/Windows/Fonts/arial.ttf") &&
        !font_.loadFromFile("C:/Windows/Fonts/consola.ttf") &&
        !font_.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf") &&
        !font_.loadFromFile("/usr/share/fonts/truetype/freefont/FreeSans.ttf")) {
        // Last resort fallback - use a basic system font
        font_.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
    }
    
    // Title
    title_text_.setFont(font_);
    title_text_.setString("Control Panel");
    title_text_.setCharacterSize(16);
    title_text_.setFillColor(sf::Color(200, 200, 200));
    
    // Labels
    vehicle_label_.setFont(font_);
    vehicle_label_.setString("Vehicles: ");
    vehicle_label_.setCharacterSize(12);
    vehicle_label_.setFillColor(sf::Color(180, 180, 180));
    
    green_label_.setFont(font_);
    green_label_.setString("Green: ");
    green_label_.setCharacterSize(12);
    green_label_.setFillColor(sf::Color(180, 180, 180));
    
    yellow_label_.setFont(font_);
    yellow_label_.setString("Yellow: ");
    yellow_label_.setCharacterSize(12);
    yellow_label_.setFillColor(sf::Color(180, 180, 180));
    
    red_label_.setFont(font_);
    red_label_.setString("Red: ");
    red_label_.setCharacterSize(12);
    red_label_.setFillColor(sf::Color(180, 180, 180));
    
    speed_label_.setFont(font_);
    speed_label_.setString("Speed: ");
    speed_label_.setCharacterSize(12);
    speed_label_.setFillColor(sf::Color(180, 180, 180));
    
    speed_value_text_.setFont(font_);
    speed_value_text_.setCharacterSize(12);
    speed_value_text_.setFillColor(sf::Color(0, 255, 100));
    
    // Button texts
    pause_text_.setFont(font_);
    pause_text_.setString("Pause");
    pause_text_.setCharacterSize(12);
    pause_text_.setFillColor(sf::Color::White);
    
    reset_text_.setFont(font_);
    reset_text_.setString("Reset");
    reset_text_.setCharacterSize(12);
    reset_text_.setFillColor(sf::Color::White);
    
    // Initialize slider tracks
    sf::Vector2f track_size(150, 8);
    sf::Color track_color(60, 60, 65);
    sf::Color slider_color(100, 100, 110);
    
    vehicle_count_track_.setSize(track_size);
    vehicle_count_track_.setFillColor(track_color);
    vehicle_count_slider_.setSize(sf::Vector2f(12, 18));
    vehicle_count_slider_.setFillColor(slider_color);
    
    green_duration_track_.setSize(track_size);
    green_duration_track_.setFillColor(track_color);
    green_duration_slider_.setSize(sf::Vector2f(12, 18));
    green_duration_slider_.setFillColor(slider_color);
    
    yellow_duration_track_.setSize(track_size);
    yellow_duration_track_.setFillColor(track_color);
    yellow_duration_slider_.setSize(sf::Vector2f(12, 18));
    yellow_duration_slider_.setFillColor(slider_color);
    
    red_duration_track_.setSize(track_size);
    red_duration_track_.setFillColor(track_color);
    red_duration_slider_.setSize(sf::Vector2f(12, 18));
    red_duration_slider_.setFillColor(slider_color);
    
    speed_track_.setSize(track_size);
    speed_track_.setFillColor(track_color);
    speed_slider_.setSize(sf::Vector2f(12, 18));
    speed_slider_.setFillColor(slider_color);
    
    // Buttons
    pause_button_.setSize(sf::Vector2f(70, 30));
    pause_button_.setFillColor(sf::Color(80, 80, 90));
    
    reset_button_.setSize(sf::Vector2f(70, 30));
    reset_button_.setFillColor(sf::Color(80, 80, 90));
}

traffic_simulation::SimulationConfig ControlPanel::getConfig() const {
    return traffic_simulation::SimulationConfig(
        12,                     // grid_size
        vehicle_count_value_,   // vehicle_count
        green_duration_value_, // green_duration
        yellow_duration_value_,// yellow_duration
        red_duration_value_    // red_duration
    );
}

bool ControlPanel::isPaused() const {
    return is_paused_;
}

bool ControlPanel::isResetRequested() const {
    return reset_requested_;
}

void ControlPanel::clearResetRequest() {
    reset_requested_ = false;
}

float ControlPanel::getSpeedMultiplier() const {
    return speed_multiplier_value_;
}

void ControlPanel::render(sf::RenderWindow& window) {
    // Panel background
    sf::RectangleShape panel_bg(sf::Vector2f(panel_width_, panel_height_));
    panel_bg.setPosition(panel_x_, panel_y_);
    panel_bg.setFillColor(sf::Color(30, 30, 35, 230));
    panel_bg.setOutlineColor(sf::Color(60, 60, 65));
    panel_bg.setOutlineThickness(2);
    window.draw(panel_bg);
    
    // Title
    title_text_.setPosition(panel_x_ + 10, panel_y_ + 10);
    window.draw(title_text_);
    
    int y_offset = 50;
    int slider_x = panel_x_ + 10;
    int track_width = 150;
    
    // Vehicle count slider
    vehicle_label_.setPosition(slider_x, panel_y_ + y_offset);
    window.draw(vehicle_label_);
    
    // Value text
    sf::Text value_text;
    value_text.setFont(font_);
    value_text.setCharacterSize(11);
    value_text.setFillColor(sf::Color(150, 150, 150));
    value_text.setString(std::to_string(vehicle_count_value_));
    value_text.setPosition(slider_x + track_width - 25, panel_y_ + y_offset);
    window.draw(value_text);
    
    vehicle_count_track_.setPosition(slider_x, panel_y_ + y_offset + 20);
    window.draw(vehicle_count_track_);
    
    // Calculate slider position
    float slider_pos = slider_x + ((vehicle_count_value_ - 20) / 180.0f) * (track_width - 12);
    vehicle_count_slider_.setPosition(slider_pos, panel_y_ + y_offset + 18);
    window.draw(vehicle_count_slider_);
    
    y_offset += 55;
    
    // Green duration slider
    green_label_.setPosition(slider_x, panel_y_ + y_offset);
    window.draw(green_label_);
    
    value_text.setString(std::to_string(green_duration_value_) + "s");
    value_text.setPosition(slider_x + track_width - 35, panel_y_ + y_offset);
    window.draw(value_text);
    
    green_duration_track_.setPosition(slider_x, panel_y_ + y_offset + 20);
    window.draw(green_duration_track_);
    
    slider_pos = slider_x + ((green_duration_value_ - 1) / 9.0f) * (track_width - 12);
    green_duration_slider_.setPosition(slider_pos, panel_y_ + y_offset + 18);
    window.draw(green_duration_slider_);
    
    y_offset += 55;
    
    // Yellow duration slider
    yellow_label_.setPosition(slider_x, panel_y_ + y_offset);
    window.draw(yellow_label_);
    
    value_text.setString(std::to_string(yellow_duration_value_) + "s");
    value_text.setPosition(slider_x + track_width - 35, panel_y_ + y_offset);
    window.draw(value_text);
    
    yellow_duration_track_.setPosition(slider_x, panel_y_ + y_offset + 20);
    window.draw(yellow_duration_track_);
    
    slider_pos = slider_x + ((yellow_duration_value_ - 1) / 4.0f) * (track_width - 12);
    yellow_duration_slider_.setPosition(slider_pos, panel_y_ + y_offset + 18);
    window.draw(yellow_duration_slider_);
    
    y_offset += 55;
    
    // Red duration slider
    red_label_.setPosition(slider_x, panel_y_ + y_offset);
    window.draw(red_label_);
    
    value_text.setString(std::to_string(red_duration_value_) + "s");
    value_text.setPosition(slider_x + track_width - 35, panel_y_ + y_offset);
    window.draw(value_text);
    
    red_duration_track_.setPosition(slider_x, panel_y_ + y_offset + 20);
    window.draw(red_duration_track_);
    
    slider_pos = slider_x + ((red_duration_value_ - 1) / 9.0f) * (track_width - 12);
    red_duration_slider_.setPosition(slider_pos, panel_y_ + y_offset + 18);
    window.draw(red_duration_slider_);
    
    y_offset += 55;
    
    // Speed slider
    speed_label_.setPosition(slider_x, panel_y_ + y_offset);
    window.draw(speed_label_);
    
    speed_value_text_.setString(std::to_string(speed_multiplier_value_) + "x");
    speed_value_text_.setPosition(slider_x + track_width - 35, panel_y_ + y_offset);
    window.draw(speed_value_text_);
    
    speed_track_.setPosition(slider_x, panel_y_ + y_offset + 20);
    window.draw(speed_track_);
    
    float speed_normalized = (speed_multiplier_value_ - 0.5f) / 2.5f;
    slider_pos = slider_x + speed_normalized * (track_width - 12);
    speed_slider_.setPosition(slider_pos, panel_y_ + y_offset + 18);
    window.draw(speed_slider_);
    
    y_offset += 60;
    
    // Pause/Resume button
    sf::Color btn_color = is_paused_ ? sf::Color(0, 180, 80) : sf::Color(180, 80, 80);
    pause_button_.setFillColor(btn_color);
    pause_button_.setPosition(slider_x, panel_y_ + y_offset);
    window.draw(pause_button_);
    
    pause_text_.setString(is_paused_ ? "Resume" : "Pause");
    pause_text_.setPosition(slider_x + 17, panel_y_ + y_offset + 8);
    window.draw(pause_text_);
    
    // Reset button
    reset_button_.setPosition(slider_x + 80, panel_y_ + y_offset);
    window.draw(reset_button_);
    reset_text_.setPosition(slider_x + 92, panel_y_ + y_offset + 8);
    window.draw(reset_text_);
    
    // Instructions at bottom
    sf::Text instructions;
    instructions.setFont(font_);
    instructions.setString("SPACE: Pause/Resume\n1/2/3: Speed\nESC: Exit");
    instructions.setCharacterSize(10);
    instructions.setFillColor(sf::Color(120, 120, 125));
    instructions.setPosition(panel_x_ + 10, panel_y_ + panel_height_ - 55);
    window.draw(instructions);
}

bool ControlPanel::handleClick(int x, int y) {
    int slider_x = panel_x_ + 10;
    int track_width = 150;
    int track_height = 30;
    
    // Check vehicle count slider
    int slider_y = panel_y_ + 70;
    if (x >= slider_x && x <= slider_x + track_width &&
        y >= slider_y && y <= slider_y + track_height) {
        float normalized = static_cast<float>(x - slider_x) / track_width;
        vehicle_count_value_ = 20 + static_cast<int>(normalized * 180);
        return true;
    }
    
    // Check green duration slider
    slider_y = panel_y_ + 125;
    if (x >= slider_x && x <= slider_x + track_width &&
        y >= slider_y && y <= slider_y + track_height) {
        float normalized = static_cast<float>(x - slider_x) / track_width;
        green_duration_value_ = 1 + static_cast<int>(normalized * 9);
        return true;
    }
    
    // Check yellow duration slider
    slider_y = panel_y_ + 180;
    if (x >= slider_x && x <= slider_x + track_width &&
        y >= slider_y && y <= slider_y + track_height) {
        float normalized = static_cast<float>(x - slider_x) / track_width;
        yellow_duration_value_ = 1 + static_cast<int>(normalized * 4);
        return true;
    }
    
    // Check red duration slider
    slider_y = panel_y_ + 235;
    if (x >= slider_x && x <= slider_x + track_width &&
        y >= slider_y && y <= slider_y + track_height) {
        float normalized = static_cast<float>(x - slider_x) / track_width;
        red_duration_value_ = 1 + static_cast<int>(normalized * 9);
        return true;
    }
    
    // Check speed slider
    slider_y = panel_y_ + 290;
    if (x >= slider_x && x <= slider_x + track_width &&
        y >= slider_y && y <= slider_y + track_height) {
        float normalized = static_cast<float>(x - slider_x) / track_width;
        speed_multiplier_value_ = 0.5f + normalized * 2.5f;
        return true;
    }
    
    // Check pause button
    if (x >= slider_x && x <= slider_x + 70 &&
        y >= panel_y_ + 320 && y <= panel_y_ + 350) {
        is_paused_ = !is_paused_;
        return true;
    }
    
    // Check reset button
    if (x >= slider_x + 80 && x <= slider_x + 150 &&
        y >= panel_y_ + 320 && y <= panel_y_ + 350) {
        reset_requested_ = true;
        return true;
    }
    
    return false;
}

} // namespace gui
