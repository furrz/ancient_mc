#pragma once
#include <vector>
#include <glm/vec3.hpp>

#include "AABB.h"
#include "BlockInfo.h"

using iv3range = std::pair<glm::ivec3, glm::ivec3>;

class Level
{
    glm::ivec3 size_;
    std::vector<uint8_t> blocks_;
    std::vector<int> lightDepths_;
    std::vector<iv3range> dirtyRegions_;
    BlockInfo *blockInfo_;

public:
    Level(int w, int h, int d, BlockInfo *blockInfo);
    bool load();
    void generate();
    void save() const;
    void setTile(glm::ivec3 pos, uint8_t blockId);
    void calcLightDepths(int xA, int zA, int xB, int zB);
    void dirty(glm::ivec3 min, glm::ivec3 max);
    void getCubes(AABB aabb, std::vector<AABB>& cubes, int attribs);

    [[nodiscard]] int blockAttribs(const glm::ivec3 pos)
    {
        return blockInfo_->attribs(block(pos));
    }

    [[nodiscard]] glm::ivec3 size() const { return size_; }

    [[nodiscard]] uint8_t& block(const glm::ivec3 pos)
    {
        return blocks_[(pos.y * size_.z + pos.z) * size_.x + pos.x];
    }

    [[nodiscard]] uint8_t block(const glm::ivec3 pos) const
    {
        return blocks_[(pos.y * size_.z + pos.z) * size_.x + pos.x];
    }

    [[nodiscard]] const std::vector<iv3range>& dirtyRegions() const
    {
        return dirtyRegions_;
    }

    void clearDirtyRegions()
    {
        dirtyRegions_.clear();
    }

    /// Check if a block coordinate is within the level bounds.
    [[nodiscard]] bool inBounds(const glm::ivec3 pos) const {
        return pos.x >= 0 && pos.y >= 0 && pos.z >= 0 && pos.x < size_.x && pos.y < size_.y && pos.z < size_.z;
    }

    [[nodiscard]] float getBrightness(const glm::ivec3 vec) const
    {
        constexpr float dark = 0.8F;
        constexpr float light = 1.0F;

        if (inBounds(vec)) {
            return vec.y < lightDepths_[vec.x + vec.z * size_.x] ? dark : light;
        }

        return light;
    }
};
