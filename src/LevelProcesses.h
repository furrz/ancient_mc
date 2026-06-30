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
        int block_beside_min, block_beside_max;
        std::optional<uint8_t> replacement_block;
        std::optional<size_t> schedule_target;
        uint8_t schedule_delay;
    };

    struct ScheduledTick
    {
        uint8_t countdown;
        glm::ivec3 pos;
        size_t process;
    };

    std::vector<RandomProcessTiming> randomProcessTiming_;
    std::vector<Process> randomProcesses_;
    std::vector<Process> tickedProcesses_;
    std::vector<Process> worldGenProcesses_;
    std::vector<Process> scheduledProcesses_;
    std::vector<ScheduledTick> scheduledTicks_;
    int tickCounter_{};

    void executeProcess(const Process& process, glm::ivec3 pos, Level *level);

public:
    LevelProcesses(ConVars *conVars, const BlockInfo *blockInfo);
    void runWorldGenProcesses(Level *level);
    void tick(Level *level, const Player *player);

};
