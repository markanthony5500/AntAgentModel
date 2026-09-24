cmake_minimum_required(VERSION 3.16)
project(AntAgentModel)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(raylib REQUIRED)

add_executable(ant_sim
    src/main.cpp src/Grid.cpp src/Nest.cpp src/Ant.cpp
    src/Simulation.cpp src/Renderer.cpp)
target_link_libraries(ant_sim PRIVATE raylib)
target_compile_options(ant_sim PRIVATE -Wall -Wextra)