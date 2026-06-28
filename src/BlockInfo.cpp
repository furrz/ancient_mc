#include "BlockInfo.h"

#include "read_json.h"

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
    const auto data = read_json("res/block_info.json");

    for (const auto& block : data) {
        const std::string id = block["id"];
        byID_[id] = id_.size();
        id_.emplace_back(id);

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
