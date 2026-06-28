#pragma once
#include <cstdint>
#include <cstddef>

namespace GZip {
    bool readFixedSize(const char *path, uint8_t *target, size_t target_size);
    bool writeFixedSize(const char *path, const uint8_t *target, size_t target_size);
}