#include "Level.h"

#include <fstream>
#include <iostream>

#include "BlockAttribs.h"

Level::Level(int w, int h, int d) : size_(w, d, h)
{
    std::ifstream in("res/block_material.txt");
    if (!in.is_open()) {
        std::cerr << "Could not load res/block_material.txt" << std::endl;
    }

    std::string token;
    while (in >> token) {
        if (token == "air")
            blockAttribs_.emplace_back(0);
        else if (token == "solid")
            blockAttribs_.emplace_back(COLLIDABLE | PICKABLE | LIGHT_BLOCKER | DRAW_OPAQUE);
        else if (token == "liquid")
            blockAttribs_.emplace_back(BUOYANT | PICKABLE | DRAW_TRANSPARENT);
        else std::cerr << "unrecognized material: " << token << std::endl;
    }
    in.close();

    blocks_.resize(w * h * d);
    lightDepths_.resize(w * h);
    if (!load()) generate();

}

bool Level::load()
{
    std::ifstream in("level.dat", std::ios::binary | std::ios::ate);

    if (!in.is_open() || in.tellg() != blocks_.size()) {
        std::cerr << "Could not load level.dat" << std::endl;
        return false;
    }

    in.seekg(0, std::ios::beg);

    in.read(
        reinterpret_cast<std::istream::char_type *>(blocks_.data()),
        blocks_.size());

    in.close();

    dirty({ 0, 0, 0 }, size_);

    return true;
}

void Level::generate()
{
    const int groundLevel = size_.y * 2 / 3;

    for (int x = 0; x < size_.x; ++x) {
        for (int y = 0; y < size_.y; ++y) {
            for (int z = 0; z < size_.z; ++z) {
                block({ x, y, z }) = y < groundLevel ? 2 : y > groundLevel ? 0 : 1;
            }
        }
    }

    dirty({ 0, 0, 0 }, size_);
}

void Level::save() const
{
    std::ofstream out("level.dat", std::ios::binary);
    out.write(
        reinterpret_cast<const std::ostream::char_type *>(blocks_.data()),
        blocks_.size());
    out.close();
}

void Level::setTile(glm::ivec3 pos, uint8_t blockId)
{
}

void Level::calcLightDepths(int xA, int zA, int xB, int zB)
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

void Level::getCubes(AABB aabb, std::vector<AABB>& cubes, int attribs)
{
    // TODO: Get Cubes with those attribs!
}
