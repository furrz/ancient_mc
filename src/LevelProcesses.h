#pragma once
#include <optional>
#include <string>
#include <vector>

class BlockInfo;
class Level;
class ConVars;
class Player;

class LevelProcesses {

    struct Process {
        std::string name;
        int interval, offset, repeat;
        uint8_t match_block;
        std::optional<uint8_t> block_above;
        bool invert_above;
        std::optional<uint8_t> block_beside;
        uint8_t replacement_block;
    };

    std::vector<Process> processes_;
    int tickCounter_{};

public:
    LevelProcesses(ConVars *conVars, const BlockInfo *blockInfo);

    void tick(Level *level, const Player *player);

};
