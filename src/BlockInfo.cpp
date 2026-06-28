#include "BlockInfo.h"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>


using json = nlohmann::json;

std::unordered_map<std::string, int> namedAttribs {
    { "LIGHT_BLOCKER", LIGHT_BLOCKER },
    { "COLLIDABLE", COLLIDABLE },
    { "BUOYANT", BUOYANT },
    { "PICKABLE", PICKABLE },
    { "DRAW_TRANSPARENT", DRAW_TRANSPARENT },
    { "DRAW_OPAQUE", DRAW_OPAQUE },
    { "FLAMMABLE", FLAMMABLE },
    { "FALLING", FALLING },
    { "DRAW_CUTOUT", DRAW_CUTOUT },
    { "FOLIAGE_COLORING", FOLIAGE_COLORING },
    { "DEFAULT", LIGHT_BLOCKER | COLLIDABLE | PICKABLE | DRAW_OPAQUE },
};

BlockInfo::BlockInfo() {
    std::ifstream in("res/block_info.json");
    if (!in.is_open()) {
        std::cerr << "Could not open res/block_info.json!" << std::endl;
        return;
    }

    const auto data = json::parse(in);
    in.close();

    for (const json& block : data) {
        name_.emplace_back(block["name"]);

        int attribs{};
        for (const std::string attribName : block["attribs"]) {
            attribs |= namedAttribs[attribName];
        }
        attribs_.emplace_back(attribs);

        uint8_t top{}, bottom{}, sides{};
        if (block.contains("all")) {
            top = bottom = sides = block["all"];
        }

        if (block.contains("top")) {
            top = block["top"];
        }

        if (block.contains("bottom")) {
            bottom = block["bottom"];
        }

        if (block.contains("sides")) {
            sides = block["sides"];
        }

        topIdx_.emplace_back(top);
        bottomIdx_.emplace_back(bottom);
        sideIdx_.emplace_back(sides);
    }
}
