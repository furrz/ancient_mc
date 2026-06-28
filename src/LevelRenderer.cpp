#include "LevelRenderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Level.h"


LevelRenderer::LevelRenderer(Level *level, BlockInfo *blockInfo)
    : level_(level),
      sizeInChunks_(level_->size() / CHUNK_SIZE),
      blockInfo_(blockInfo) {
    const int chunkCount = numChunks();
    chunksDirty_.resize(chunkCount);
    chunkDrawLists_ = glGenLists(4 * chunkCount);

    // Load terrain image
    int w, h, nChannels;
    const auto data = stbi_load("res/terrain.png", &w, &h, &nChannels, 4);

    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_2D, texture_);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, 0x84FE /* anisotropy ext */, 4.0f);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
}

LevelRenderer::~LevelRenderer() {
    glDeleteLists(chunkDrawLists_, numChunks() * 4);
}

void LevelRenderer::rebuildChunk(const glm::ivec3 pos) {
    const auto index = chunkIndex(pos);
    chunksDirty_[index] = false;

    const auto start = pos * 16;
    const auto end = start + 16;

    for (const auto layer: {0, 1}) {
        for (const auto transparent: {false, true}) {
            const auto attribMask = transparent ? DRAW_TRANSPARENT : DRAW_OPAQUE;

            const int offset = layer + (transparent ? 2 : 0);

            glNewList(chunkDrawLists_ + index * 4 + offset, GL_COMPILE);
            glEnable(GL_TEXTURE_2D);
            // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glBindTexture(GL_TEXTURE_2D, texture_);
            glBegin(GL_QUADS);

            for (int x = start.x; x < end.x; x++) {
                for (int y = start.y; y < end.y; y++) {
                    for (int z = start.z; z < end.z; z++) {
                        if (level_->blockAttribs({x, y, z}) & attribMask) {
                            drawTile({x, y, z}, layer, attribMask);
                        }
                    }
                }
            }

            glEnd();
            glEndList();
        }
    }
}

inline bool checkFace(Level *level, const glm::ivec3 pos, const glm::ivec3 offset,
    const int attribMask, const float c, const int layer,
                      float &br) {
    const auto check = pos + offset;

    if (!level->inBounds(check)) {
        br = c;
        return layer == 0;
    }

    if (!(level->blockAttribs(check) & attribMask)) {
        const float brightness = level->getBrightness(check);
        br = brightness * c;
        return (brightness == 1) == (layer == 0);
    }

    return false;
}

constexpr glm::vec2 uvMinFor(const uint8_t tex) {
    return glm::vec2{
        static_cast<float>(tex % 16) / 16.0f,
        static_cast<float>(tex / 16) / 16.0f
    };
}

void LevelRenderer::drawTile(const glm::ivec3 pos, int layer, int attribMask) const {
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

    if (checkFace(level_, pos, {0, -1, 0}, attribMask, c1, layer, br)) {
        const auto tex = blockInfo_->textureIndex(level_->block(pos), 0);
        const auto uvMin = uvMinFor(tex);
        const auto uvMax = uvMin + 0.0624375f;

        glColor4f(br, br, br, 1.0f);
        glTexCoord2f(uvMin.x, uvMax.y);
        glVertex3f(x0, y0, z1);
        glTexCoord2f(uvMin.x, uvMin.y);
        glVertex3f(x0, y0, z0);
        glTexCoord2f(uvMax.x, uvMin.y);
        glVertex3f(x1, y0, z0);
        glTexCoord2f(uvMax.x, uvMax.y);
        glVertex3f(x1, y0, z1);
    }

    if (checkFace(level_, pos, {0, 1, 0}, attribMask, c1, layer, br)) {
        const auto tex = blockInfo_->textureIndex(level_->block(pos), 1);
        const auto uvMin = uvMinFor(tex);
        const auto uvMax = uvMin + 0.0624375f;

        if (blockInfo_->attribs(level_->block(pos)) & FOLIAGE_COLORING) {
            glColor4f(br * 140.0f / 255.0f, br * 250.0f / 255.0f, br * 130.0f / 255.0f, 1.0f);
        } else {
            glColor4f(br, br, br, 1.0f);
        }

        glTexCoord2f(uvMax.x, uvMax.y);
        glVertex3f(x1, y1, z1);
        glTexCoord2f(uvMax.x, uvMin.y);
        glVertex3f(x1, y1, z0);
        glTexCoord2f(uvMin.x, uvMin.y);
        glVertex3f(x0, y1, z0);
        glTexCoord2f(uvMin.x, uvMax.y);
        glVertex3f(x0, y1, z1);
    }

    if (checkFace(level_, pos, {0, 0, -1}, attribMask, c2, layer, br)) {
        const auto tex = blockInfo_->textureIndex(level_->block(pos), 2);
        const auto uvMin = uvMinFor(tex);
        const auto uvMax = uvMin + 0.0624375f;

        glColor4f(br, br, br, 1.0f);
        glTexCoord2f(uvMax.x, uvMin.y);
        glVertex3f(x0, y1, z0);
        glTexCoord2f(uvMin.x, uvMin.y);
        glVertex3f(x1, y1, z0);
        glTexCoord2f(uvMin.x, uvMax.y);
        glVertex3f(x1, y0, z0);
        glTexCoord2f(uvMax.x, uvMax.y);
        glVertex3f(x0, y0, z0);
    }

    if (checkFace(level_, pos, {0, 0, 1}, attribMask, c2, layer, br)) {
        const auto tex = blockInfo_->textureIndex(level_->block(pos), 3);
        const auto uvMin = uvMinFor(tex);
        const auto uvMax = uvMin + 0.0624375f;

        glColor4f(br, br, br, 1.0f);
        glTexCoord2f(uvMin.x, uvMin.y);
        glVertex3f(x0, y1, z1);
        glTexCoord2f(uvMin.x, uvMax.y);
        glVertex3f(x0, y0, z1);
        glTexCoord2f(uvMax.x, uvMax.y);
        glVertex3f(x1, y0, z1);
        glTexCoord2f(uvMax.x, uvMin.y);
        glVertex3f(x1, y1, z1);
    }

    if (checkFace(level_, pos, {-1, 0, 0}, attribMask, c3, layer, br)) {
        const auto tex = blockInfo_->textureIndex(level_->block(pos), 4);
        const auto uvMin = uvMinFor(tex);
        const auto uvMax = uvMin + 0.0624375f;

        glColor4f(br, br, br, 1.0f);
        glTexCoord2f(uvMax.x, uvMin.y);
        glVertex3f(x0, y1, z1);
        glTexCoord2f(uvMin.x, uvMin.y);
        glVertex3f(x0, y1, z0);
        glTexCoord2f(uvMin.x, uvMax.y);
        glVertex3f(x0, y0, z0);
        glTexCoord2f(uvMax.x, uvMax.y);
        glVertex3f(x0, y0, z1);
    }

    if (checkFace(level_, pos, {1, 0, 0}, attribMask, c3, layer, br)) {
        const auto tex = blockInfo_->textureIndex(level_->block(pos), 5);
        const auto uvMin = uvMinFor(tex);
        const auto uvMax = uvMin + 0.0624375f;

        glColor4f(br, br, br, 1.0f);
        glTexCoord2f(uvMin.x, uvMax.y);
        glVertex3f(x1, y0, z1);
        glTexCoord2f(uvMax.x, uvMax.y);
        glVertex3f(x1, y0, z0);
        glTexCoord2f(uvMax.x, uvMin.y);
        glVertex3f(x1, y1, z0);
        glTexCoord2f(uvMin.x, uvMin.y);
        glVertex3f(x1, y1, z1);
    }
}


void LevelRenderer::render(const Player *player) {
    static std::vector<glm::ivec3> visibleChunks;
    visibleChunks.clear();

    // Todo: Frustum Cull
    for (int x = 0; x < sizeInChunks_.x; x++) {
        for (int z = 0; z < sizeInChunks_.z; z++) {
            for (int y = 0; y < sizeInChunks_.y; y++) {
                visibleChunks.emplace_back(x, y, z);
            }
        }
    }

    // Rebuild exactly one visible chunk
    int updatedCount = 0;
    for (const auto pos: visibleChunks) {
        if (chunksDirty_[chunkIndex(pos)]) {
            rebuildChunk(pos);
            updatedCount++;
            break;
        }
    }

    if (updatedCount == 0 && prevUpdated_ != 0) {
        std::cout << "Done chunk updates!" << std::endl;
    } else if (updatedCount != 0 && prevUpdated_ == 0) {
        std::cout << "Starting chunk updates..." << std::endl;
    }

    prevUpdated_ = updatedCount;

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
        glCallList(chunkDrawLists_ + chunkIndex(pos) * 4 + 3);
    }

    // Pass 4: Brightly Lit Transparent
    glDisable(GL_FOG);
    for (const auto pos: visibleChunks) {
        glCallList(chunkDrawLists_ + chunkIndex(pos) * 4 + 2);
    }
}


void renderFace(int x, int y, int z, int face) {
    float x0 = (float) x + 0.0F;
    float x1 = (float) x + 1.0F;
    float y0 = (float) y + 0.0F;
    float y1 = (float) y + 1.0F;
    float z0 = (float) z + 0.0F;
    float z1 = (float) z + 1.0F;
    if (face == 0) {
        glVertex3f(x0, y0, z1);
        glVertex3f(x0, y0, z0);
        glVertex3f(x1, y0, z0);
        glVertex3f(x1, y0, z1);
    }

    if (face == 1) {
        glVertex3f(x1, y1, z1);
        glVertex3f(x1, y1, z0);
        glVertex3f(x0, y1, z0);
        glVertex3f(x0, y1, z1);
    }

    if (face == 2) {
        glVertex3f(x0, y1, z0);
        glVertex3f(x1, y1, z0);
        glVertex3f(x1, y0, z0);
        glVertex3f(x0, y0, z0);
    }

    if (face == 3) {
        glVertex3f(x0, y1, z1);
        glVertex3f(x0, y0, z1);
        glVertex3f(x1, y0, z1);
        glVertex3f(x1, y1, z1);
    }

    if (face == 4) {
        glVertex3f(x0, y1, z1);
        glVertex3f(x0, y1, z0);
        glVertex3f(x0, y0, z0);
        glVertex3f(x0, y0, z1);
    }

    if (face == 5) {
        glVertex3f(x1, y0, z1);
        glVertex3f(x1, y0, z0);
        glVertex3f(x1, y1, z0);
        glVertex3f(x1, y1, z1);
    }
}


void LevelRenderer::renderHit(const HitResult &value) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glColor4f(1.0f, 1.0f, 1.0f, sinf(glfwGetTime()) * 0.2f + 0.4f);
    glBegin(GL_QUADS);
    renderFace(value.pos.x, value.pos.y, value.pos.z, value.f);
    glEnd();
    glDisable(GL_BLEND);
}

void LevelRenderer::pick(const Player *player) {
    float r = 3.0F;
    const auto [a, b] = player->box().grown({r, r, r});

    glInitNames();

    for (int x = static_cast<int>(a.x); x < static_cast<int>(b.x) + 1; ++x) {
        if (x < 0 || x >= level_->size().x) continue;
        glPushName(x);

        for (int y = static_cast<int>(a.y); y < static_cast<int>(b.y) + 1; ++y) {
            if (y < 0 || y >= level_->size().y) continue;
            glPushName(y);

            for (int z = static_cast<int>(a.z); z < static_cast<int>(b.z) + 1; ++z) {
                if (z < 0 || z >= level_->size().z) continue;

                glPushName(z);
                if (level_->blockAttribs({x, y, z}) & PICKABLE) {
                    glPushName(0);

                    for (int i = 0; i < 6; ++i) {
                        glPushName(i);
                        glBegin(GL_QUADS);
                        renderFace(x, y, z, i);
                        glEnd();
                        glPopName();
                    }

                    glPopName();
                }

                glPopName();
            }

            glPopName();
        }

        glPopName();
    }
}

void LevelRenderer::handleDirtyRegions() {
    for (const auto &regions = level_->dirtyRegions();
         const auto [low, high]: regions) {
        const auto lowChunk = glm::max(low / CHUNK_SIZE, {0, 0, 0});
        const auto highChunk = glm::min(high / CHUNK_SIZE, sizeInChunks_ - glm::ivec3{1, 1, 1});

        for (int x = lowChunk.x; x <= highChunk.x; x++) {
            for (int y = lowChunk.y; y <= highChunk.y; y++) {
                for (int z = lowChunk.z; z <= highChunk.z; z++) {
                    chunksDirty_[chunkIndex({x, y, z})] = true;
                }
            }
        }
    }

    level_->clearDirtyRegions();
}
