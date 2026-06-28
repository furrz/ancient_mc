#pragma once
#include <optional>
#include <string>
#include <vector>
#include <glm/vec3.hpp>

class BlockInfo;
class Level;
class ConVars;
class Player;

class LevelProcesses {

    struct RandomProcessTiming {
        int interval, offset, repeat;
    };

    struct Process {
        std::string name;
        uint8_t match_block;
        std::optional<uint8_t> block_above;
        bool invert_above;
        std::optional<uint8_t> block_beside;
        uint8_t replacement_block;
    };

    std::vector<RandomProcessTiming> randomProcessTiming_;
    std::vector<Process> randomProcesses_;
    std::vector<Process> tickedProcesses_;
    int tickCounter_{};

    void executeProcess(const Process& process, glm::ivec3 pos, Level *level, const Player *player);

public:
    LevelProcesses(ConVars *conVars, const BlockInfo *blockInfo);

    void tick(Level *level, const Player *player);

};
