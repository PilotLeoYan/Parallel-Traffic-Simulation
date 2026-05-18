/**
 * @file simulation_window.cpp
 * @brief Light-minimalist GUI for the city traffic simulation.
 *
 * Design decisions:
 *  - Warm cream background (#F5F3EE) for city blocks
 *  - Warm gray road surface  (#C4C2BD)
 *  - Bold colored squares at each intersection for traffic lights
 *  - Clean white toolbar at top: title + status dot + Pause/Continue/Next Step buttons
 *  - Subtle dark direction arrows on roads
 */

#include "gui/simulation_window.hpp"
#include "gui/renderer.hpp"
#include "common/simulation_controller.hpp"

#include <city/city.hpp>
#include <city/street.hpp>
#include <traffic/semaphore_controller.hpp>

namespace gui {

// ----------------------------------------------------------------
// Constructor / Destructor
// ----------------------------------------------------------------

SimulationWindow::SimulationWindow()
    : city_(nullptr),
      vehicles_(nullptr),
      semaphores_(nullptr),
      cell_size_(40),
      speed_multiplier_(1.0f),
      is_paused_(false),
      window_closed_(false),
      font_loaded_(false),
      controller_(nullptr) {

    // Light minimalist palette
    background_color_   = sf::Color(245, 243, 238);   // warm cream
    grid_color_         = sf::Color(220, 218, 213);   // subtle grid line
    street_color_       = sf::Color(196, 194, 189);   // warm gray road
    intersection_color_ = sf::Color(196, 194, 189);   // neutral (no semaphore)

    // Vivid traffic-light colors
    green_light_  = sf::Color( 34, 197,  94);
    yellow_light_ = sf::Color(234, 179,   8);
    red_light_    = sf::Color(239,  68,  68);
}

SimulationWindow::~SimulationWindow() {
    close();
}

// ----------------------------------------------------------------
// initialize
// ----------------------------------------------------------------

bool SimulationWindow::initialize(int width, int height, const std::string& title) {
    window_.create(sf::VideoMode(width, height), title);
    window_.setFramerateLimit(60);

    // Try to load a font for toolbar text; fall back gracefully if none found
    font_loaded_ =
        font_.loadFromFile("C:/Windows/Fonts/segoeui.ttf")   ||
        font_.loadFromFile("C:/Windows/Fonts/arial.ttf")     ||
        font_.loadFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf") ||
        font_.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")                 ||
        font_.loadFromFile("/usr/share/fonts/truetype/freefont/FreeSans.ttf");

    return window_.isOpen();
}

// ----------------------------------------------------------------
// Setters
// ----------------------------------------------------------------

void SimulationWindow::setCity(city::City* city)                              { city_      = city;      }
void SimulationWindow::setVehicles(std::vector<VehicleRenderInfo>* vehicles)  { vehicles_  = vehicles;  }
void SimulationWindow::setSemaphoreController(traffic::SemaphoreController* s){ semaphores_= s;         }
void SimulationWindow::setCellSize(int cell_size)                             { cell_size_ = cell_size; }
void SimulationWindow::setSpeedMultiplier(float m)                            { speed_multiplier_ = m;  }

bool SimulationWindow::isOpen()   const { return window_.isOpen() && !window_closed_; }
bool SimulationWindow::isPaused() const { return is_paused_; }
void SimulationWindow::togglePause()              { is_paused_ = !is_paused_; }
void SimulationWindow::setPaused(bool p)          { is_paused_ = p; }

// ----------------------------------------------------------------
// handleEvents -- keyboard + mouse button click on toolbar buttons
// ----------------------------------------------------------------

void SimulationWindow::handleEvents() {
    sf::Event event;
    while (window_.pollEvent(event)) {

        // --- Window close ---
        if (event.type == sf::Event::Closed) {
            window_closed_ = true;
            window_.close();
        }

        // --- Mouse click on toolbar buttons ---
        else if (event.type == sf::Event::MouseButtonPressed &&
                 event.mouseButton.button == sf::Mouse::Left) {
            float mx = static_cast<float>(event.mouseButton.x);
            float my = static_cast<float>(event.mouseButton.y);

            if (pause_btn_bounds_.contains(mx, my)) {
                if (controller_) controller_->pause();
                is_paused_ = true;
            }
            else if (continue_btn_bounds_.contains(mx, my)) {
                if (controller_) controller_->resume();
                is_paused_ = false;
            }
            else if (step_btn_bounds_.contains(mx, my)) {
                // stepOnce() auto-pauses internally; sync local flag
                if (controller_) controller_->stepOnce();
                is_paused_ = true;
            }
        }

        // --- Keyboard shortcuts ---
        else if (event.type == sf::Event::KeyPressed) {
            switch (event.key.code) {
                case sf::Keyboard::Space:
                    if (is_paused_) {
                        if (controller_) controller_->resume();
                        is_paused_ = false;
                    } else {
                        if (controller_) controller_->pause();
                        is_paused_ = true;
                    }
                    break;
                case sf::Keyboard::Num1: setSpeedMultiplier(1.0f); break;
                case sf::Keyboard::Num2: setSpeedMultiplier(2.0f); break;
                case sf::Keyboard::Num3: setSpeedMultiplier(3.0f); break;
                case sf::Keyboard::Escape:
                    window_closed_ = true;
                    window_.close();
                    break;
                default: break;
            }
        }

        // --- Window resize ---
        else if (event.type == sf::Event::Resized) {
            sf::FloatRect area(0.f, 0.f,
                               static_cast<float>(event.size.width),
                               static_cast<float>(event.size.height));
            window_.setView(sf::View(area));
        }
    }
}

// ----------------------------------------------------------------
// render -- master draw call
// ----------------------------------------------------------------

void SimulationWindow::render() {
    window_.clear(background_color_);

    if (city_) {
        drawGrid();
        drawStreets();
        drawStreetNames();
        drawIntersections();   // bold colored squares
        drawStreetDirections();
        drawVehicles();
    }

    drawToolbar();  // always rendered on top
    window_.display();
}

// ----------------------------------------------------------------
// drawGrid -- fills the grid area with warm cream
// ----------------------------------------------------------------

void SimulationWindow::drawGrid() {
    int gs   = city_->getGridSize();
    float tw = static_cast<float>(gs * cell_size_);
    float th = static_cast<float>(gs * cell_size_);
    float tb = static_cast<float>(TOOLBAR_HEIGHT);

    sf::RectangleShape bg(sf::Vector2f(tw, th));
    bg.setPosition(0.f, tb);
    bg.setFillColor(background_color_);
    window_.draw(bg);
}

// ----------------------------------------------------------------
// drawStreets -- warm gray road strips + subtle center dashes
// ----------------------------------------------------------------

void SimulationWindow::drawStreets() {
    int   gs     = city_->getGridSize();
    float total  = static_cast<float>(gs * cell_size_);
    float road_w = cell_size_ * 0.60f;
    float offset = (cell_size_ - road_w) * 0.5f;
    float tb     = static_cast<float>(TOOLBAR_HEIGHT);

    sf::Color asphalt(196, 194, 189);
    sf::Color dash   (140, 138, 133, 140);

    // --- Horizontal strips ---
    for (int y = 0; y < gs; ++y) {
        float py = y * cell_size_ + offset + tb;

        sf::RectangleShape road(sf::Vector2f(total, road_w));
        road.setPosition(0.f, py);
        road.setFillColor(asphalt);
        window_.draw(road);

        // Center-line dashes (skip inside intersection zone)
        float cy = y * cell_size_ + cell_size_ * 0.5f + tb;
        for (int i = 0; i < static_cast<int>(total); i += 14) {
            int lx = i % cell_size_;
            if (lx > static_cast<int>(offset) &&
                lx < static_cast<int>(cell_size_ - offset)) continue;
            sf::RectangleShape d(sf::Vector2f(8.f, 2.f));
            d.setPosition(static_cast<float>(i), cy - 1.f);
            d.setFillColor(dash);
            window_.draw(d);
        }
    }

    // --- Vertical strips ---
    for (int x = 0; x < gs; ++x) {
        float px = x * cell_size_ + offset;

        sf::RectangleShape road(sf::Vector2f(road_w, total));
        road.setPosition(px, tb);
        road.setFillColor(asphalt);
        window_.draw(road);

        // Center-line dashes
        float cx = x * cell_size_ + cell_size_ * 0.5f;
        for (int i = 0; i < static_cast<int>(total); i += 14) {
            int ly = i % cell_size_;
            if (ly > static_cast<int>(offset) &&
                ly < static_cast<int>(cell_size_ - offset)) continue;
            sf::RectangleShape d(sf::Vector2f(2.f, 8.f));
            d.setPosition(cx - 1.f, static_cast<float>(i) + tb);
            d.setFillColor(dash);
            window_.draw(d);
        }
    }
}

// ----------------------------------------------------------------
// drawIntersections -- BOLD colored square per intersection
// Color encodes current traffic-light state (or neutral gray if none)
// ----------------------------------------------------------------

void SimulationWindow::drawIntersections() {
    int   gs     = city_->getGridSize();
    float road_w = cell_size_ * 0.60f;
    float offset = (cell_size_ - road_w) * 0.5f;
    float tb     = static_cast<float>(TOOLBAR_HEIGHT);

    for (int y = 0; y < gs; ++y) {
        for (int x = 0; x < gs; ++x) {
            float px = x * cell_size_ + offset;
            float py = y * cell_size_ + offset + tb;

            sf::Color c = getTrafficLightColor(x, y);

            // Outer square (the full intersection area)
            sf::RectangleShape sq(sf::Vector2f(road_w, road_w));
            sq.setPosition(px, py);
            sq.setFillColor(c);
            window_.draw(sq);

            // Small dark inner mark so no-light intersections still read clearly
            if (c == intersection_color_) {
                float dot_r = road_w * 0.18f;
                sf::RectangleShape dot(sf::Vector2f(dot_r, dot_r));
                dot.setPosition(px + (road_w - dot_r) * 0.5f,
                                py + (road_w - dot_r) * 0.5f);
                dot.setFillColor(sf::Color(160, 158, 153));
                window_.draw(dot);
            }
        }
    }
}

// ----------------------------------------------------------------
// drawStreetDirections -- subtle dark arrows painted on the road
// ----------------------------------------------------------------

void SimulationWindow::drawStreetDirections() {
    if (!city_) return;

    const auto& streets = city_->getStreets();
    sf::Color arrow_color(85, 83, 79, 120);
    float arrow_size = cell_size_ * 0.17f;
    float tb         = static_cast<float>(TOOLBAR_HEIGHT);

    for (const auto& street : streets) {
        auto start = street->getStartIntersection()->getCoordinate();
        auto end   = street->getEndIntersection()->getCoordinate();

        float sx = start.x * cell_size_ + cell_size_ * 0.5f;
        float sy = start.y * cell_size_ + cell_size_ * 0.5f + tb;
        float ex = end.x   * cell_size_ + cell_size_ * 0.5f;
        float ey = end.y   * cell_size_ + cell_size_ * 0.5f + tb;

        float cx = (sx + ex) * 0.5f;
        float cy = (sy + ey) * 0.5f;

        city::Direction dir = street->getDirection();
        float angle = 0.f;
        float dx = 0.f, dy = 0.f;

        if      (dir == city::Direction::EAST)  { angle =   0.f; dy =  cell_size_ * 0.14f; }
        else if (dir == city::Direction::SOUTH)  { angle =  90.f; dx = -cell_size_ * 0.14f; }
        else if (dir == city::Direction::WEST)   { angle = 180.f; dy = -cell_size_ * 0.14f; }
        else if (dir == city::Direction::NORTH)  { angle = -90.f; dx =  cell_size_ * 0.14f; }

        if (street->isTwoWay()) {
            Renderer::drawArrow(window_, cx + dx, cy + dy, angle,         arrow_size * 0.6f, arrow_color);
            Renderer::drawArrow(window_, cx - dx, cy - dy, angle + 180.f, arrow_size * 0.6f, arrow_color);
        } else {
            Renderer::drawArrow(window_, cx, cy, angle, arrow_size, arrow_color);
        }
    }
}

// ----------------------------------------------------------------
// drawStreetNames -- street names on axes
// ----------------------------------------------------------------
void SimulationWindow::drawStreetNames() {
    if (!city_ || !font_loaded_) return;
    float tb = static_cast<float>(TOOLBAR_HEIGHT);
    int gs = city_->getGridSize();

    // Group horizontal streets by Y, vertical by X
    std::map<int, std::string> h_names; // y -> name
    std::map<int, std::string> v_names; // x -> name

    for (const auto& s : city_->getStreets()) {
        auto dir = s->getDirection();
        auto start = s->getStartIntersection()->getCoordinate();
        if (dir == city::Direction::EAST || dir == city::Direction::WEST) {
            if (h_names.find(start.y) == h_names.end())
                h_names[start.y] = s->getName();
        } else {
            if (v_names.find(start.x) == v_names.end())
                v_names[start.x] = s->getName();
        }
    }

    // Horizontal street names — left of the grid
    for (auto& [y, name] : h_names) {
        sf::Text t;
        t.setFont(font_);
        t.setString(name);
        t.setCharacterSize(static_cast<unsigned int>(cell_size_ * 0.2f));
        t.setFillColor(sf::Color(80, 78, 74));
        float py = y * cell_size_ + cell_size_ * 0.5f + tb;
        t.setPosition(gs * cell_size_ + 3.f, py - 5.f);
        window_.draw(t);
    }

    // Vertical street names — below the grid, rotated 90°
    for (auto& [x, name] : v_names) {
        sf::Text t;
        t.setFont(font_);
        t.setString(name);
        t.setCharacterSize(static_cast<unsigned int>(cell_size_ * 0.2f));
        t.setFillColor(sf::Color(80, 78, 74));
        t.setRotation(90.f);
        float px = x * cell_size_ + cell_size_ * 0.5f;
        float py = gs * cell_size_ + tb + 3.f;
        t.setPosition(px + 5.f, py);
        window_.draw(t);
    }
}

// ----------------------------------------------------------------
// drawVehicles -- path preview lines, then vehicle bodies
// ----------------------------------------------------------------

void SimulationWindow::drawVehicles() {
    if (!vehicles_) return;

    float tb = static_cast<float>(TOOLBAR_HEIGHT);

    // 1. Path preview (drawn below vehicles)
    for (const auto& v : *vehicles_) {
        if (v.remaining_path.size() > 1) {
            sf::VertexArray line(sf::LineStrip,
                                 static_cast<unsigned>(v.remaining_path.size()) + 1u);

            sf::Color c = Renderer::getVehicleColor(v.id);
            c.a = 90;

            float sx = v.x * cell_size_ + cell_size_ * 0.5f;
            float sy = v.y * cell_size_ + cell_size_ * 0.5f + tb;
            line[0].position = sf::Vector2f(sx, sy);
            line[0].color    = c;

            for (std::size_t i = 0; i < v.remaining_path.size(); ++i) {
                float px = v.remaining_path[i].x * cell_size_ + cell_size_ * 0.5f;
                float py = v.remaining_path[i].y * cell_size_ + cell_size_ * 0.5f + tb;
                line[i + 1u].position = sf::Vector2f(px, py);
                line[i + 1u].color    = c;
            }
            window_.draw(line);
        }
    }

    // Draw destination markers first (so they appear below vehicles)
    for (const auto& v : *vehicles_) {
        float dx = v.destination_x * cell_size_ + cell_size_ * 0.5f;
        float dy = v.destination_y * cell_size_ + cell_size_ * 0.5f + tb;

        sf::CircleShape dest_dot(5.f);
        dest_dot.setOrigin(5.f, 5.f);
        dest_dot.setPosition(dx, dy);
        sf::Color vc = Renderer::getVehicleColor(v.id);
        dest_dot.setFillColor(sf::Color(vc.r, vc.g, vc.b, 180));
        dest_dot.setOutlineColor(sf::Color(30, 28, 25));
        dest_dot.setOutlineThickness(1.2f);
        window_.draw(dest_dot);

        if (font_loaded_) {
            sf::Text lbl;
            lbl.setFont(font_);
            lbl.setString(std::to_string(v.id));
            lbl.setCharacterSize(static_cast<unsigned int>(cell_size_ * 0.2f));
            lbl.setFillColor(sf::Color(30, 28, 25));
            sf::FloatRect lb = lbl.getLocalBounds();
            lbl.setPosition(dx - lb.width * 0.5f - lb.left, dy - lb.height * 0.5f - lb.top);
            window_.draw(lbl);
        }
    }

    // 2. Vehicle bodies
    for (const auto& v : *vehicles_) {
        float px = v.x * cell_size_ + cell_size_ * 0.5f;
        float py = v.y * cell_size_ + cell_size_ * 0.5f + tb;
        Renderer::drawVehicle(window_, px, py,
                              cell_size_ / 3.0f,
                              v.id, v.is_waiting, v.angle);
        // Draw vehicle ID on top of each car
        if (font_loaded_) {
            sf::Text id_text;
            id_text.setFont(font_);
            id_text.setString(std::to_string(v.id));
            id_text.setCharacterSize(static_cast<unsigned int>(cell_size_ * 0.225f));
            id_text.setStyle(sf::Text::Bold);
            id_text.setFillColor(sf::Color(255, 255, 255));
            id_text.setOutlineColor(sf::Color(0, 0, 0));
            id_text.setOutlineThickness(1.f);
            sf::FloatRect lb = id_text.getLocalBounds();
            id_text.setPosition(px - lb.width * 0.5f - lb.left,
                                py - lb.height * 0.5f - lb.top);
            window_.draw(id_text);
        }
    }
}

// ----------------------------------------------------------------
// drawToolbar -- white top bar: status dot | title | Pause | Continue | Next Step
// ----------------------------------------------------------------

void SimulationWindow::drawToolbar() {
    float win_w = static_cast<float>(window_.getSize().x);
    float tb    = static_cast<float>(TOOLBAR_HEIGHT);

    // --- Background ---
    sf::RectangleShape bg(sf::Vector2f(win_w, tb));
    bg.setPosition(0.f, 0.f);
    bg.setFillColor(sf::Color(255, 255, 255));
    window_.draw(bg);

    // --- Bottom separator ---
    sf::RectangleShape sep(sf::Vector2f(win_w, 1.f));
    sep.setPosition(0.f, tb - 1.f);
    sep.setFillColor(sf::Color(210, 208, 203));
    window_.draw(sep);

    // --- Status indicator dot (green = running, red = paused) ---
    sf::CircleShape dot(5.f);
    dot.setOrigin(5.f, 5.f);
    dot.setPosition(14.f, 15.f);
    dot.setFillColor(is_paused_ ? sf::Color(239, 68, 68) : sf::Color(34, 197, 94));
    window_.draw(dot);

    // --- Title and status text ---
    if (font_loaded_) {
        sf::Text title;
        title.setFont(font_);
        title.setString(controller_->isSimulationComplete() ?
            "City Traffic Simulation [Finished]" :
            "City Traffic Simulation");
        title.setCharacterSize(static_cast<unsigned int>(cell_size_ * 0.4f));
        title.setStyle(sf::Text::Bold);
        title.setFillColor(sf::Color(35, 33, 30));
        title.setPosition(26.f, 7.f);
        window_.draw(title);

        sf::Text status;
        status.setFont(font_);
        status.setString(is_paused_ ?
            "PAUSED  -  press SPACE or Continue to resume" :
            controller_->isSimulationComplete() ? "COMPLETED" : "RUNNING  -  press SPACE to pause");
        status.setCharacterSize(static_cast<unsigned int>(cell_size_ * 0.25f));
        status.setFillColor(sf::Color(130, 128, 123));
        status.setPosition(26.f, 30.f);
        window_.draw(status);
    }

    // --- Three buttons: [Pause] [Continue] [Next Step] ---
    const float BTN_W    = 108.f;
    const float BTN_H    = 34.f;
    const float GAP      = 8.f;
    const float R_PAD    = 14.f;
    float btn_y = (tb - BTN_H) * 0.5f;

    float step_x  = win_w - R_PAD - BTN_W;
    float cont_x  = step_x - GAP - BTN_W;
    float pause_x = cont_x - GAP - BTN_W;

    // Store for click detection (updated every frame so resize works)
    pause_btn_bounds_    = sf::FloatRect(pause_x, btn_y, BTN_W, BTN_H);
    continue_btn_bounds_ = sf::FloatRect(cont_x,  btn_y, BTN_W, BTN_H);
    step_btn_bounds_     = sf::FloatRect(step_x,  btn_y, BTN_W, BTN_H);

    struct BtnDef { float x; sf::Color col; const char* label; };
    BtnDef btns[3] = {
        { pause_x,  sf::Color(230, 142,   8), "Pause"     },
        { cont_x,   sf::Color( 14, 170, 118), "Continue"  },
        { step_x,   sf::Color( 50, 118, 230), "Next Step" }
    };

    for (const auto& b : btns) {
        // Button body
        sf::RectangleShape rect(sf::Vector2f(BTN_W, BTN_H));
        rect.setPosition(b.x, btn_y);
        rect.setFillColor(b.col);
        window_.draw(rect);

        // Subtle top highlight stripe
        sf::RectangleShape highlight(sf::Vector2f(BTN_W, 3.f));
        highlight.setPosition(b.x, btn_y);
        highlight.setFillColor(sf::Color(255, 255, 255, 50));
        window_.draw(highlight);

        // Button label
        if (font_loaded_) {
            sf::Text lbl;
            lbl.setFont(font_);
            lbl.setString(b.label);
            lbl.setCharacterSize(static_cast<unsigned int>(cell_size_ * 0.325f));
            lbl.setStyle(sf::Text::Bold);
            lbl.setFillColor(sf::Color(255, 255, 255));

            // Center text inside the button
            sf::FloatRect lb = lbl.getLocalBounds();
            lbl.setPosition(
                b.x + (BTN_W - lb.width)  * 0.5f - lb.left,
                btn_y + (BTN_H - lb.height) * 0.5f - lb.top - 1.f
            );
            window_.draw(lbl);
        }
    }
}

// ----------------------------------------------------------------
// getTrafficLightColor
// ----------------------------------------------------------------

sf::Color SimulationWindow::getTrafficLightColor(int x, int y) {
    if (!semaphores_) return intersection_color_;

    city::Coordinate coord{x, y};
    const auto* sem = semaphores_->getSemaphoreAt(coord);

    if (sem) {
        switch (sem->getState()) {
            case traffic::TrafficLightState::GREEN:  return green_light_;
            case traffic::TrafficLightState::YELLOW: return yellow_light_;
            case traffic::TrafficLightState::RED:    return red_light_;
        }
    }
    return intersection_color_;
}

// ----------------------------------------------------------------
// close / run
// ----------------------------------------------------------------

void SimulationWindow::close() {
    if (window_.isOpen()) window_.close();
}

void SimulationWindow::run(traffic_simulation::SimulationController& controller) {
    controller_ = &controller;
    std::vector<gui::VehicleRenderInfo> render_info;

    while (isOpen()) {
        handleEvents();

        render_info = controller.getVehicleManager().getRenderInfo();
        setVehicles(&render_info);

        render();
        sf::sleep(sf::milliseconds(16));
    }
}

} // namespace gui
