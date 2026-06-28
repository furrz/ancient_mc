#pragma once
#include <vector>

#include "BlockInfo.h"


class Inventory
{
    std::vector<uint8_t> slots_ {};
    uint8_t slot_ {};

    BlockInfo *blockInfo_;

public:
    explicit Inventory(BlockInfo *blockInfo);;
    void tick();
    [[nodiscard]] uint8_t getBlockId() const;
    void render();
};
