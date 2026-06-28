#include "Level.h"

#include <noise/noise.h>
#include <leveldb/db.h>

#include "ConVars.h"
#include "read_json.h"

Level::Level(ConVars *conVars, const int w, const int h, const int d, BlockInfo *blockInfo) : size_(w, d, h), blockInfo_(blockInfo)
{
    conVars->setupVar("level_gen_force", "Force level regeneration even if a save file is found", &attrForceRegen_);
    blocks_.resize(w * h * d);
    lightDepths_.resize(w * h);
}

void Level::init() {
    leveldb::DB *db_tmp;
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, "level.db", &db_tmp);

    db_ = std::unique_ptr<leveldb::DB>(db_tmp);

    if (attrForceRegen_ || !load()) generate();

    if (attrForceRegen_) {
        std::cout << "Reminder: You have level_gen_force enabled! Saved level will be overwritten!" << std::endl;
    }
}

bool Level::load()
{
    leveldb::ReadOptions readOptions;
    readOptions.fill_cache = false;
    std::string level_data;
    if (!db_->Get(readOptions, "whole_world", &level_data).ok())
        return false;

    memcpy(blocks_.data(), level_data.data(), blocks_.size() * sizeof(blocks_[0]));

    dirty({ 0, 0, 0 }, size_);

    return true;
}

void Level::generate()
{
    struct Layer {
        uint8_t block;
        int y;
        float noiseAmount;
        float noiseScale;
        bool skippable;
    };

    std::vector<Layer> layers;

    const auto config = read_json("res/worldgen.json");
    for (const auto& layer : config["layers"]) {
        layers.emplace_back(
            blockInfo_->byID(layer.value<std::string>("block", "air")),
            layer.value<int>("y", 999999),
            layer.value<float>("noise", 0),
            layer.value<float>("noiseScale", 1),
            layer.value<bool>("skippable", false));
    }

    // shaping: stone or air?
    // water fill
    // surface layer - grass, sand, dirt
    // features


    noise::module::Perlin perlin;
    perlin.SetOctaveCount(8);
    perlin.SetLacunarity(0.5f);


    for (int x = 0; x < size_.x; ++x) {
        for (int z = 0; z < size_.z; ++z) {
            int layer = -1;
            int nextY = -1;

            for (int y = 0; y < size_.y; ++y) {
                while (y > nextY) {
                    layer++;
                    nextY = layers[layer].y + static_cast<int>((perlin.GetValue(x * layers[layer].noiseScale, layer * layers[layer].noiseScale, z * layers[layer].noiseScale) + 1) / 2 * layers[layer].noiseAmount);
                    if (!layers[layer].skippable) break;
                }
                block({ x, y, z }) = layers[layer].block;
            }
        }
    }

    dirty({ 0, 0, 0 }, size_);
    justGenerated_ = true;
}

void Level::save() const
{
    db_->Put(leveldb::WriteOptions(), "whole_world",
        leveldb::Slice(reinterpret_cast<const char*>(blocks_.data()), blocks_.size()));
}

void Level::setTile(const glm::ivec3 pos, const uint8_t blockId)
{
    block(pos) = blockId;
    tickTile(pos);
    tickTile(pos + glm::ivec3 { 1, 0, 0 });
    tickTile(pos + glm::ivec3 { -1, 0, 0 });
    tickTile(pos + glm::ivec3 { 0, 1, 0 });
    tickTile(pos + glm::ivec3 { 0, -1, 0 });
    tickTile(pos + glm::ivec3 { 0, -0, 1 });
    tickTile(pos + glm::ivec3 { 0, -0, -1 });

    dirty(glm::max(pos - 1, glm::ivec3{ 0, 0, 0 }), glm::min(pos + 1, size_ - 1));
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
