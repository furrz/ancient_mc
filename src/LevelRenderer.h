#pragma once

#include <vector>

#include "HitResult.h"
#include "Player.h"

class Level;

constexpr auto CHUNK_SIZE = 16;


class LevelRenderer
{
    Level *level_;
    glm::ivec3 sizeInChunks_;
    std::vector<bool> chunksDirty_;
    unsigned texture_;
    int chunkDrawLists_;
    int prevUpdated_{};

    void drawTile(glm::ivec3 pos, int layer, int attribMask) const;

public:
    explicit LevelRenderer(Level *level);
    ~LevelRenderer();
    void rebuildChunk(const glm::ivec3 pos);
    void render(const Player *player);
    void renderHit(const HitResult& value);
    void pick(const Player *player);
    void handleDirtyRegions();

    [[nodiscard]] int chunkIndex(const glm::ivec3 pos) const
    {
        return (pos.y * sizeInChunks_.z + pos.z) * sizeInChunks_.x + pos.x;
    }

    [[nodiscard]] int numChunks() const
    {
        return sizeInChunks_.x * sizeInChunks_.y * sizeInChunks_.z;
    }
};