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

    const auto getOptional = [&](const std::string& key) {
        return std::make_optional<uint8_t>(blockInfo->byID(key));
    };

    for (const auto& process : data["processes"]) {
        processes_.emplace_back(
            process.value<std::string>("name", "unnamed"),
            process.value<int>("interval", 1),
            process.value<int>("offset", 0),
            process.value<int>("repeat", 1),
            blockInfo->byID(process.value<std::string>("block", "air")),
            process.value<std::optional<std::string>>("above", std::nullopt).and_then(getOptional),
            process.value<bool>("invert_above", false),
            process.value<std::optional<std::string>>("beside", std::nullopt).and_then(getOptional),
            blockInfo->byID(process.value<std::string>("set_block", "air"))
        );
    }
}

bool isBeside(Level *level, glm::ivec3 pos, uint8_t block) {
    for (const auto side : { glm::ivec3(1, 0, 0), glm::ivec3(-1, 0, 0), glm::ivec3(0, 0, 1), glm::ivec3(0, 0, -1)}) {
        const auto p = pos + side;
        if (level->inBounds(p) && level->block(p) == block) return true;
    }

    return false;
}

void LevelProcesses::tick(Level *level, const Player *player) {
    for (const auto& process : processes_) {
        if ((tickCounter_ + process.offset) % process.interval) continue;

        for (int i = 0; i < process.repeat; i++) {
            const auto random_pos = glm::ivec3(player->pos() + glm::vec3{ (RNG::randomFloat() * 80.0f) - 40.0f, (RNG::randomFloat() * 40.0f) - 20.0f, (RNG::randomFloat() * 80.0f) - 40.0f });
            if (!level->inBounds(random_pos)) continue;

            const auto block = level->block(random_pos);
            if (block != process.match_block) continue;

            const auto above = random_pos + glm::ivec3(0, 1, 0);
            if (process.block_above && (!level->inBounds(above) || ((level->block(above) == process.block_above.value()) == process.invert_above)))
                continue;

            if (process.block_beside && !isBeside(level, random_pos, process.block_beside.value()))
                continue;

            std::cout << "Process happened: " << process.name << std::endl;

            level->setTile(random_pos, process.replacement_block);
        }
    }

    tickCounter_++;
}
