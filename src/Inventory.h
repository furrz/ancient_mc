#pragma once
#include <vector>


class Inventory
{
    std::vector<uint8_t> slots_ {
        1, 2, 3
    };

    uint8_t slot_ {};

public:
    void tick();
    [[nodiscard]] uint8_t getBlockId() const;
    void render();
};
