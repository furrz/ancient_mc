#pragma once

#include <vector>

#include "BlockInfo.h"
#include "HitResult.h"
#include "Player.h"

class Level;

constexpr auto CHUNK_SIZE = 16;


class LevelRenderer
{
    Level *level_;
    glm::ivec3 sizeInChunks_;
    std::vector<bool> chunksDirty_;
    unsigned texture_{};
    int chunkDrawLists_;
    BlockInfo *blockInfo_;
    int attrMaxChunkRebuildsPerFrame_ = 4;

    void drawTile(glm::ivec3 pos, int layer, int attribMask) const;

public:
    explicit LevelRenderer(ConVars *conVars, Level *level, BlockInfo *blockInfo);
    ~LevelRenderer();
    void rebuildChunk(glm::ivec3 pos);
    void render(const Player *player);
    void renderHit(const HitResult& value);
    void pick(const Player *player) const;
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