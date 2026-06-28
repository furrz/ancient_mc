#pragma once
#include <ctime>
#include <cstdlib>

namespace RNG {
    inline void seed()
    {
        srand(time(nullptr));
    }

    [[nodiscard]] inline float randomFloat()
    {
        return static_cast<float>(static_cast<double>(std::rand()) / RAND_MAX);
    }
}
