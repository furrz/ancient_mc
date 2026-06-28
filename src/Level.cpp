#include "Level.h"

#include <fstream>
#include <iostream>

#include "GZip.h"

Level::Level(const int w, const int h, const int d, BlockInfo *blockInfo) : size_(w, d, h), blockInfo_(blockInfo)
{
    blocks_.resize(w * h * d);
    lightDepths_.resize(w * h);
    if (!load()) generate();

}

bool Level::load()
{
    if (!GZip::readFixedSize("level.dat", blocks_.data(), blocks_.size())) {
        return false;
    }

    dirty({ 0, 0, 0 }, size_);

    return true;
}

void Level::generate()
{
    const int groundLevel = size_.y * 2 / 3;

    for (int x = 0; x < size_.x; ++x) {
        for (int y = 0; y < size_.y; ++y) {
            for (int z = 0; z < size_.z; ++z) {
                block({ x, y, z }) = y < groundLevel - 2 ? 1 : y < groundLevel ? 2 : y > groundLevel ? 0 : 3;
            }
        }
    }

    dirty({ 0, 0, 0 }, size_);
}

void Level::save() const
{
    GZip::writeFixedSize("level.dat", blocks_.data(), blocks_.size());
}

void Level::setTile(const glm::ivec3 pos, const uint8_t blockId)
{
    block(pos) = blockId;
    dirty(pos - 1, pos + 1);
}

void Level::calcLightDepths(const int xA, const int zA, const int xB, const int zB)
{
    for(int x = xA; x < xB; ++x) {
        for(int z = zA; z < zB; ++z) {
            int y;
            for(y = size_.y - 1; y > 0 && !(blockAttribs({ x, y, z }) & LIGHT_BLOCKER); --y) {
            }

            lightDepths_[x + z * size_.x] = y;
        }
    }
}

void Level::dirty(glm::ivec3 min, glm::ivec3 max)
{
    calcLightDepths(min.x, min.z, max.x, max.z);
    dirtyRegions_.emplace_back(min, max);
}

void Level::getCubes(const AABB aabb, std::vector<AABB>& cubes, const int attribs)
{
    for(int x = std::max(0, static_cast<int>(aabb.a.x) - 1); x <= std::min(static_cast<int>(aabb.b.x + 1), size_.x - 1); ++x) {
        for (int y = std::max(0, static_cast<int>(aabb.a.y) - 1); y <= std::min(static_cast<int>(aabb.b.y + 1), size_.y - 1); ++y) {
            for (int z = std::max(0, static_cast<int>(aabb.a.z) - 1); z <= std::min(static_cast<int>(aabb.b.z) + 1, size_.z - 1); ++z) {
                if (blockAttribs({ x, y, z }) & attribs)
                    cubes.emplace_back(
                        glm::vec3 { x, y, z },
                        glm::vec3 { x + 1, y + 1, z + 1 });
            }
        }
    }
}
