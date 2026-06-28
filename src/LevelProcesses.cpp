//
// Created by tague on 1/11/2026.
//

#include "LevelProcesses.h"
#include "ConVars.h"
#include "BlockInfo.h"
#include "Level.h"
#include "Player.h"
#include "read_json.h"
#include "rng.h"

LevelProcesses::LevelProcesses(ConVars *conVars, const BlockInfo *blockInfo) {
    const auto data = read_json("res/processes.json");

    const auto getOptional = [&](const std::string &key) {
        return std::make_optional<uint8_t>(blockInfo->byID(key));
    };

    for (const auto &process: data["processes"]) {
        const auto types = process.value<nlohmann::json>("types",
            nlohmann::json::array({ "random" }));

        const auto proc = Process{
            process.value<std::string>("name", "unnamed"),
            blockInfo->byID(process.value<std::string>("block", "air")),
            process.value<std::optional<std::string> >("above", std::nullopt).and_then(getOptional),
            process.value<bool>("invert_above", false),
            process.value<std::optional<std::string> >("beside", std::nullopt).and_then(getOptional),
            process.value<int>("beside_min", 1),
            process.value<int>("beside_max", 5),
            blockInfo->byID(process.value<std::string>("set_block", "air"))
        };

        for (const auto& type_name : types) {
            if (type_name == "random") {
                randomProcessTiming_.emplace_back(
                    process.value<int>("interval", 1),
                    process.value<int>("offset", 0),
                    process.value<int>("repeat", 1)
                );
                randomProcesses_.emplace_back(proc);
            } else if (type_name == "ticked") {
                tickedProcesses_.emplace_back(proc);
            } else if (type_name == "world_gen") {
                worldGenProcesses_.emplace_back(proc);
            } else {
                std::cerr << "Unrecognized process type: " << type_name << std::endl;
            }
        }
    }
}

void LevelProcesses::runWorldGenProcesses(Level *level) {
    const auto size = level->size();
    for (const auto& proc : worldGenProcesses_)
        for (int x = 0; x < size.x; x++)
            for (int z = 0; z < size.z; z++)
                for (int y = 0; y < size.y; y++)
                    executeProcess(proc, { x, y, z }, level);
}

int horizontalNeighborCount(Level *level, glm::ivec3 pos, uint8_t block) {
    int count = 0;
    for (const auto side: {glm::ivec3(1, 0, 0), glm::ivec3(-1, 0, 0), glm::ivec3(0, 0, 1), glm::ivec3(0, 0, -1)}) {
        const auto p = pos + side;
        if (level->inBounds(p) && level->block(p) == block) count++;
    }

    return count;
}


void LevelProcesses::tick(Level *level, const Player *player) {
    for (int i = 0; i < randomProcessTiming_.size(); ++i) {
        const auto [interval, offset, repeat] = randomProcessTiming_.at(i);
        if ((tickCounter_ + offset) % interval) continue;

        const auto& process = randomProcesses_.at(i);

        for (int j = 0; j < repeat; j++) {
            const auto random_pos = glm::ivec3(player->pos() + glm::vec3{
                                                   (RNG::randomFloat() * 80.0f) - 40.0f,
                                                   (RNG::randomFloat() * 40.0f) - 20.0f,
                                                   (RNG::randomFloat() * 80.0f) - 40.0f
                                               });
            executeProcess(process, random_pos, level);
        }
    }

    for (const auto& process : tickedProcesses_) {
        for (const auto pos : level->tickedBlocks()) {
            executeProcess(process, pos, level);
        }
    }

    tickCounter_++;
}

void LevelProcesses::executeProcess(const Process &process, glm::ivec3 pos, Level *level) {
    if (!level->inBounds(pos)) return;

    const auto block = level->block(pos);
    if (block != process.match_block) return;

    const auto above = pos + glm::ivec3(0, 1, 0);
    if (process.block_above && (!level->inBounds(above) || (
                                    (level->block(above) == process.block_above.value()) == process.
                                    invert_above)))
        return;


    if (process.block_beside) {
        const auto count = horizontalNeighborCount(level, pos, process.block_beside.value());
        if (count < process.block_beside_min || count > process.block_beside_max)
            return;
    }

    level->setTile(pos, process.replacement_block);
}
