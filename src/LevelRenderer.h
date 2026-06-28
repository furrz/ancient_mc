#pragma once

#include <vector>

#include "AABB.h"
#include "BlockInfo.h"
#include "ConVars.h"
#include "HitResult.h"

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
    int attrRenderDistance_ = 8;

    void drawTile(glm::ivec3 pos, int attribMask) const;

public:
    explicit LevelRenderer(ConVars *conVars, Level *level, BlockInfo *blockInfo);
    ~LevelRenderer();
    void rebuildChunk(glm::ivec3 pos);
    void render(glm::ivec3 center);
    void renderHit(const HitResult& value);
    void pick(const AABB& box) const;
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