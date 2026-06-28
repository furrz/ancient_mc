#pragma once
#include <string>
#include <unordered_map>
#include <vector>

enum BlockAttribs
{
    LIGHT_BLOCKER = 1,
    COLLIDABLE = 2,
    BUOYANT = 4,
    PICKABLE = 8,
    DRAW_TRANSPARENT = 16,
    DRAW_OPAQUE = 32,
    FLAMMABLE = 64,
    FALLING = 128,
    DRAW_CUTOUT = 256,
    FOLIAGE_COLORING = 512
};

class BlockInfo {
    std::vector<std::string> name_;
    std::vector<uint8_t> sideIdx_;
    std::vector<uint8_t> topIdx_;
    std::vector<uint8_t> bottomIdx_;
    std::vector<int> attribs_;
    std::vector<std::string> id_;
    std::unordered_map<std::string, uint8_t> byID_;

public:
    BlockInfo();

    [[nodiscard]] int attribs(const uint8_t block) const {
        return attribs_[block];
    }

    [[nodiscard]] const std::string& name(const uint8_t block) const {
        return name_[block];
    }

    [[nodiscard]] const std::string& id(const uint8_t block) const {
        return id_[block];
    }

    [[nodiscard]] uint8_t byID(const std::string& id) const {
        return byID_.at(id);
    }

    [[nodiscard]] uint8_t textureIndex(const uint8_t block, const int face) const {
        switch (face) {
            case 0: return bottomIdx_[block];
            case 1: return topIdx_[block];
            default: return sideIdx_[block];
        }
    }
};
