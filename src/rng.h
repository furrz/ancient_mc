#pragma once
#include <ctime>
#include <cstdlib>

namespace RNG {
    inline void seed()
    {
        srand(time(nullptr)); // NOLINT(*-msc50-cpp)
    }

    [[nodiscard]] inline float randomFloat()
    {
        return static_cast<float>(static_cast<double>(std::rand()) / RAND_MAX); // NOLINT(*-msc50-cpp)
    }

    [[nodiscard]] inline int randomInt()
    {
        return std::rand(); // NOLINT(*-msc50-cpp)
    }
}
