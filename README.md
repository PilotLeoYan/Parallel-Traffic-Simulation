# Traffic Simulation

A high-performance traffic simulation system demonstrating concurrency patterns, pathfinding algorithms, and parallel computing. Built with C++17, SFML for visualization, and OpenMP for parallel processing.

## Features

- **Concurrent Traffic Management**: Thread-safe vehicle movement and traffic light state management using mutexes and condition variables
- **A\* Pathfinding**: Efficient shortest-path computation for vehicle routing through the city grid
- **OpenMP Parallelization**: Parallel processing for vehicle updates and traffic light cycling
- **Interactive GUI**: Real-time visualization with SFML showing vehicles, traffic lights, and road network

## Requirements

- C++17 or later
- CMake 3.10+
- [SFML](https://www.sfml-dev.org/) 2.5+
- OpenMP (GCC on Linux, MSVC on Windows)

### Installing Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get install cmake libsfml-dev libomp-dev
```

**Windows (vcpkg):**
```bash
# Install vcpkg first, then:
vcpkg install sfml:x64-windows
vcpkg integrate install
```

## Building

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

The compiled executable will be placed in the `build` directory.

### Building on Windows

**Using vcpkg (recommended):**
```bash
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

**Using Visual Studio:**
```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

## Usage

```bash
./traffic_simulation [options]
```

### Command Line Options

| Option | Description | Default |
|--------|-------------|---------|
| `--no-gui` | Run simulation without GUI (headless mode) | `false` |
| `--vehicles <N>` | Number of vehicles in simulation | `10` |
| `--grid <N>` | Grid size (N x N intersections) | `5` |
| `--green <N>` | Green light duration (seconds) | `5` |
| `--yellow <N>` | Yellow light duration (seconds) | `2` |
| `--red <N>` | Red light duration (seconds) | `5` |

### Examples

Run with default settings:
```bash
./traffic_simulation
```

Run headless with 20 vehicles on a 7x7 grid:
```bash
./traffic_simulation --no-gui --vehicles 20 --grid 7
```

Custom traffic light timings:
```bash
./traffic_simulation --green 8 --yellow 3 --red 10
```

## Project Structure

```
Parallel-Traffic-Simulation/
├── CMakeLists.txt              # Build configuration (cross-platform)
├── tests/
│   ├── CMakeLists.txt
│   ├── test_main.cpp
│   ├── city_test.cpp
│   ├── common_types_test.cpp
│   ├── pathfinder_test.cpp
│   ├── traffic_test.cpp
│   ├── vehicle_test.cpp
│   └── test_framework.hpp
├── include/
│   ├── city/
│   │   ├── city.hpp
│   │   ├── intersection.hpp
│   │   └── street.hpp
│   ├── common/
│   │   ├── constants.hpp
│   │   ├── logger.hpp
│   │   ├── simulation_controller.hpp
│   │   └── types.hpp
│   ├── gui/
│   │   ├── control_panel.hpp
│   │   ├── renderer.hpp
│   │   └── simulation_window.hpp
│   ├── monitoring/
│   │   ├── metrics.hpp
│   │   └── metrics_collector.hpp
│   ├── traffic/
│   │   ├── semaphore.hpp
│   │   └── semaphore_controller.hpp
│   └── vehicle/
│       ├── pathfinder.hpp
│       ├── vehicle.hpp
│       └── vehicle_manager.hpp
└── src/
    ├── main.cpp
    ├── city/
    │   ├── city.cpp
    │   ├── intersection.cpp
    │   └── street.cpp
    ├── common/
    │   ├── logger.cpp
    │   ├── simulation_controller.cpp
    │   └── types.cpp
    ├── gui/
    │   ├── control_panel.cpp
    │   ├── renderer.cpp
    │   └── simulation_window.cpp
    ├── monitoring/
    │   ├── metrics.cpp
    │   └── metrics_collector.cpp
    ├── traffic/
    │   ├── semaphore.cpp
    │   └── semaphore_controller.cpp
    └── vehicle/
        ├── pathfinder.cpp
        ├── vehicle.cpp
        └── vehicle_manager.cpp
```

1. **CityGrid** manages the road network and intersections
2. **TrafficLight** objects coordinate vehicle flow at intersections using condition variables
3. **Vehicle** objects navigate using A* pathfinding
4. **Simulation** orchestrates the parallel execution with OpenMP

For a detailed architecture diagram, see [docs/scheme.drawio](docs/scheme.drawio).

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

Please ensure code follows existing style and includes appropriate tests.
