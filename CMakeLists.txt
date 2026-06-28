cmake_minimum_required(VERSION 4.0)
project(ancient_mc)

set(CMAKE_CXX_STANDARD 23)

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
        src/LevelRenderer.cpp
        src/LevelRenderer.h
        src/HitResult.h
        src/BlockInfo.cpp
        src/BlockInfo.h
        src/read_json.h
        src/rng.h
        src/ConVars.cpp
        src/ConVars.h
)

find_package(glm CONFIG REQUIRED)
target_link_libraries(ancient_mc PRIVATE glm::glm)

find_package(glad CONFIG REQUIRED)
target_link_libraries(ancient_mc PRIVATE glad::glad)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(ancient_mc PRIVATE glfw)

find_package(Stb REQUIRED)
target_include_directories(ancient_mc PRIVATE ${Stb_INCLUDE_DIR})

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(ancient_mc PRIVATE nlohmann_json::nlohmann_json)