#include "GZip.h"

#include <iostream>
#include <ostream>
#include <zlib.h>

bool GZip::readFixedSize(const char *path, uint8_t *target, const size_t target_size)
{
    gzFile file = gzopen(path, "rb");

    if (file == nullptr) {
        std::cerr << "Failed to read " << path << std::endl;
        return false;
    }

    int bytes_read;
    size_t total_read = 0;

    while ((bytes_read = gzread(file, target + total_read, target_size - total_read)) > 0) {
        total_read += bytes_read;
    }

    if (total_read > target_size) {
        std::cerr << "GZip Read too Much!" << std::endl;
    } else if (total_read < target_size) {
        std::cerr << "GZip Read too Little!" << std::endl;
    }

    if (bytes_read < 0) {
        std::cerr << "GZip Error reading " << path <<": " << gzerror(file, nullptr) << std::endl;
        gzclose(file);
        return false;
    }

    gzclose(file);
    return true;
}

bool GZip::writeFixedSize(const char *path, const uint8_t *target, size_t target_size)
{
    gzFile file = gzopen(path, "wb");

    if (file == nullptr) {
        std::cerr << "Failed to open for writing: " << path << std::endl;
        return false;
    }

    if (gzwrite(file, target, target_size) != target_size) {
        std::cerr << "GZip Error Writing " << path << ": " << gzerror(file, nullptr) << std::endl;
        gzclose(file);
        return false;
    }


    if (gzclose(file) != Z_OK) {
        std::cerr << "Gzip failed to close after writing: " << path << std::endl;
        return false;
    }

    return true;
}
