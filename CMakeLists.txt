cmake_minimum_required(VERSION 3.10)
project(traffic_simulation VERSION 1.0 LANGUAGES CXX)

# Configuraciones de C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

# Buscar OpenMP
find_package(OpenMP REQUIRED)

# Buscar Threads (cross-platform, replaces hardcoded pthread)
find_package(Threads REQUIRED)

# Try to find SFML via CMake first (works on Windows with vcpkg/manual installs)
# If not found, fall back to pkg-config (for Unix systems without CMake config files)
find_package(SFML 2.5 COMPONENTS graphics window system)
if(NOT SFML_FOUND)
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(SFML_PKGCONF REQUIRED IMPORTED_TARGET sfml-graphics sfml-window sfml-system)
    endif()
endif()

# Add source files from all src/* directories
file(GLOB_RECURSE SOURCES 
    "src/*.cpp" "src/*/*.cpp"
)

# Include directories
set(INCLUDE_DIRS
    include
    include/city
    include/traffic
    include/vehicle
    include/monitoring
    include/gui
    include/common
)

# Crear el ejecutable
add_executable(traffic_simulation ${SOURCES})
target_include_directories(traffic_simulation PRIVATE ${INCLUDE_DIRS})

# Enlace DIRECTO de las librerías (sin intermediarios que inyecten -mwindows)
if(SFML_FOUND)
    # Use imported targets from CMake find_package (SFML 2.x uses sfml-graphics style names)
    target_link_libraries(traffic_simulation PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
        Threads::Threads
        OpenMP::OpenMP_CXX
    )
elseif(SFML_PKGCONF_FOUND)
    # Fall back to pkg-config modules (Unix systems without CMake config files)
    target_link_libraries(traffic_simulation PRIVATE
        PkgConfig::SFML_PKGCONF
        Threads::Threads
        OpenMP::OpenMP_CXX
    )
else()
    message(FATAL_ERROR "SFML not found. Please install SFML via your package manager or CMake config.")
endif()

# Windows-specific settings
if(MSVC)
    # Use MD runtime (DLL) and enable multi-processor compilation
    target_compile_options(traffic_simulation PRIVATE /MD /MP)
elseif(MINGW)
    # For MinGW, force console subsystem (prevents -mwindows default)
    target_link_options(traffic_simulation PRIVATE "-mconsole")
endif()
