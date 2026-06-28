cmake_minimum_required(VERSION 4.1)
project(ancient_mc)

set(CMAKE_CXX_STANDARD 20)

add_executable(ancient_mc src/main.cpp
        src/Level.cpp
        src/Level.h
        src/Player.cpp
        src/Player.h
        src/Inventory.cpp
        src/Inventory.h
        src/Input.cpp
        src/Input.h
        src/AABB.h
        src/BlockAttribs.h
        src/LevelRenderer.cpp
        src/LevelRenderer.h
        src/HitResult.h)

find_package(glm CONFIG REQUIRED)
target_link_libraries(ancient_mc PRIVATE glm::glm)

find_package(glad CONFIG REQUIRED)
target_link_libraries(ancient_mc PRIVATE glad::glad)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(ancient_mc PRIVATE glfw)
