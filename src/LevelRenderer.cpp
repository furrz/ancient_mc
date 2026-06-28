#include "LevelRenderer.h"

#include <iostream>
#include <glad/glad.h>

#include "Level.h"


LevelRenderer::LevelRenderer(Level *level) : level_(level), sizeInChunks_(level_->size() / CHUNK_SIZE)
{
    const int chunkCount = numChunks();
    chunksDirty_.resize(chunkCount);
    chunkDrawLists_ = glGenLists(4 * chunkCount);
}

LevelRenderer::~LevelRenderer()
{
    glDeleteLists(chunkDrawLists_, numChunks() * 4);
}

void LevelRenderer::rebuildChunk(const glm::ivec3 pos)
{
    const auto index = chunkIndex(pos);
    chunksDirty_[index] = false;

    for (const auto layer: { 0, 1 }) {
        for (const auto transparent: { false, true }) {
            const auto attribMask = transparent ? DRAW_TRANSPARENT : DRAW_OPAQUE;

            glNewList(chunkDrawLists_ + index * 4 + layer + (transparent ? 0 : 2), GL_COMPILE);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texture_);
            glBegin(GL_QUADS);

            for (int x = pos.x * 16; x < pos.x * 16 + 16; x++) {
                for (int y = pos.y * 16; y < pos.y * 16 + 16; y++) {
                    for (int z = pos.z * 16; z < pos.z * 16 + 16; z++) {
                        if (level_->blockAttribs({ x, y, z }) & attribMask) {
                            drawTile({ x, y, z }, layer, attribMask);
                        }
                    }
                }
            }

            glEnd();
            glEndList();
        }
    }
}

inline bool checkFace(Level *level, const glm::ivec3 pos, const glm::ivec3 offset, int attribMask, float c, int layer,
                      float& br)
{
    const auto check = pos + offset;
    if (!(level->blockAttribs(check) & attribMask)) {
        br = level->getBrightness(check) * c;
        return br == c ^ layer == 1;
    }

    return false;
}

void LevelRenderer::drawTile(const glm::ivec3 pos, int layer, int attribMask) const
{
    const auto tex = level_->block(pos);

    const float u0 = static_cast<float>(tex) / 16.0F;
    const float u1 = u0 + 0.0624375F;
    constexpr float v0 = 0.0F;
    constexpr float v1 = v0 + 0.0624375F;
    constexpr float c1 = 1.0F;
    constexpr float c2 = 0.8F;
    constexpr float c3 = 0.6F;
    const float x0 = static_cast<float>(pos.x) + 0.0F;
    const float x1 = static_cast<float>(pos.x) + 1.0F;
    const float y0 = static_cast<float>(pos.y) + 0.0F;
    const float y1 = static_cast<float>(pos.y) + 1.0F;
    const float z0 = static_cast<float>(pos.z) + 0.0F;
    const float z1 = static_cast<float>(pos.z) + 1.0F;
    float br;

    if (checkFace(level_, pos, { 0, -1, 0 }, attribMask, c1, layer, br)) {
        glColor4f(br, br, br, 1.0f);
        glTexCoord2f(u0, v1);
        glVertex3f(x0, y0, z1);
        glTexCoord2f(u0, v0);
        glVertex3f(x0, y0, z0);
        glTexCoord2f(u1, v0);
        glVertex3f(x1, y0, z0);
        glTexCoord2f(u1, v1);
        glVertex3f(x1, y0, z1);
    }

    if (checkFace(level_, pos, { 0, 1, 0 }, attribMask, c1, layer, br)) {
        glColor4f(br, br, br, 1.0f);
        glTexCoord2f(u1, v1);
        glVertex3f(x1, y1, z1);
        glTexCoord2f(u1, v0);
        glVertex3f(x1, y1, z0);
        glTexCoord2f(u0, v0);
        glVertex3f(x0, y1, z0);
        glTexCoord2f(u0, v1);
        glVertex3f(x0, y1, z1);
    }

    if (checkFace(level_, pos, { 0, 0, -1 }, attribMask, c2, layer, br)) {
        glColor4f(br, br, br, 1.0f);
        glTexCoord2f(u1, v0);
        glVertex3f(x0, y1, z0);
        glTexCoord2f(u0, v0);
        glVertex3f(x1, y1, z0);
        glTexCoord2f(u0, v1);
        glVertex3f(x1, y0, z0);
        glTexCoord2f(u1, v1);
        glVertex3f(x0, y0, z0);
    }

    if (checkFace(level_, pos, { 0, 0, 1 }, attribMask, c2, layer, br)) {
        glColor4f(br, br, br, 1.0f);
        glTexCoord2f(u0, v0);
        glVertex3f(x0, y1, z1);
        glTexCoord2f(u0, v1);
        glVertex3f(x0, y0, z1);
        glTexCoord2f(u1, v1);
        glVertex3f(x1, y0, z1);
        glTexCoord2f(u1, v0);
        glVertex3f(x1, y1, z1);
    }

    if (checkFace(level_, pos, { -1, 0, 0 }, attribMask, c3, layer, br)) {
        glColor4f(br, br, br, 1.0f);
        glTexCoord2f(u1, v0);
        glVertex3f(x0, y1, z1);
        glTexCoord2f(u0, v0);
        glVertex3f(x0, y1, z0);
        glTexCoord2f(u0, v1);
        glVertex3f(x0, y0, z0);
        glTexCoord2f(u1, v1);
        glVertex3f(x0, y0, z1);
    }

    if (checkFace(level_, pos, { 1, 0, 0 }, attribMask, c3, layer, br)) {
        glColor4f(br, br, br, 1.0f);
        glTexCoord2f(u0, v1);
        glVertex3f(x1, y0, z1);
        glTexCoord2f(u1, v1);
        glVertex3f(x1, y0, z0);
        glTexCoord2f(u1, v0);
        glVertex3f(x1, y1, z0);
        glTexCoord2f(u0, v0);
        glVertex3f(x1, y1, z1);
    }
}


void LevelRenderer::render(const Player *player)
{
    static std::vector<glm::ivec3> visibleChunks;
    visibleChunks.clear();

    // Todo: Frustum Cull
    for (int x = 0; x < sizeInChunks_.x; x++) {
        for (int y = 0; y < sizeInChunks_.y; y++) {
            for (int z = 0; z < sizeInChunks_.z; z++) {
                visibleChunks.emplace_back(glm::ivec3{ x, y, z });
            }
        }
    }

    // Rebuild exactly one visible chunk
    for (const auto pos: visibleChunks) {
        const auto index = chunkIndex(pos);
        if (!chunksDirty_[index]) break;

        rebuildChunk(pos);
    }

    // Pass 1: Brightly Lit Opaque
    glDisable(GL_FOG);
    for (const auto pos: visibleChunks) {
        glCallList(chunkDrawLists_ + chunkIndex(pos) * 4);
    }

    // Pass 2: Foggy Opaque
    glEnable(GL_FOG);
    for (const auto pos: visibleChunks) {
        glCallList(chunkDrawLists_ + chunkIndex(pos) * 4 + 1);
    }

    // Pass 3: Foggy Transparent
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (const auto pos: visibleChunks) {
        glCallList(chunkDrawLists_ + chunkIndex(pos) * 4 + 2);
    }

    // Pass 4: Brightly Lit Transparent
    glDisable(GL_FOG);
    for (const auto pos: visibleChunks) {
        glCallList(chunkDrawLists_ + chunkIndex(pos) * 4 + 3);
    }
}

void LevelRenderer::renderHit(const HitResult& value)
{
}

void LevelRenderer::pick(const Player *player)
{
}

void LevelRenderer::handleDirtyRegions()
{
    for (const auto& regions = level_->dirtyRegions();
         const auto [low, high]: regions) {
        const auto lowChunk = glm::max(low / CHUNK_SIZE, { 0, 0, 0 });
        const auto highChunk = glm::min(high / CHUNK_SIZE, sizeInChunks_ - glm::ivec3{ 1, 1, 1 });

        for (int x = lowChunk.x; x < highChunk.x; x++) {
            for (int y = lowChunk.y; y < highChunk.y; y++) {
                for (int z = lowChunk.z; z < highChunk.z; z++) {
                    chunksDirty_[chunkIndex({ x, y, z })] = true;
                }
            }
        }
    }

    level_->clearDirtyRegions();
}
