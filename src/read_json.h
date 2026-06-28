#pragma once

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

inline nlohmann::json read_json(const char *path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        std::cerr << "Failed to read: " << path << std::endl;
        return {};
    }

    return nlohmann::json::parse(in);
}