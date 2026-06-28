#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "BlockAttribs.h"

class BlockInfo {
    std::vector<std::string> name_;
    std::vector<uint8_t> sideIdx_;
    std::vector<uint8_t> topIdx_;
    std::vector<uint8_t> bottomIdx_;
    std::vector<int> attribs_;

public:
    BlockInfo();

    [[nodiscard]] int attribs(const uint8_t block) const {
        return attribs_[block];
    }

    [[nodiscard]] const std::string& name(const uint8_t block) const {
        return name_[block];
    }

    [[nodiscard]] uint8_t textureIndex(const uint8_t block, const int face) const {
        switch (face) {
            case 0: return bottomIdx_[block];
            case 1: return topIdx_[block];
            default: return sideIdx_[block];
        }
    }
};
