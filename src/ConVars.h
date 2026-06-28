#pragma once
#include <map>
#include <string>

namespace cv_internal {
    enum class CVType
    {
        Float,
        Int,
        Bool
    };

    template<typename T> constexpr CVType getType() = delete;
    template<> constexpr CVType getType<float>() { return CVType::Float; }
    template<> constexpr CVType getType<int>() { return CVType::Int; }
    template<> constexpr CVType getType<bool>() { return CVType::Bool; }
}


class ConVars
{

    struct Entry
    {
        const char *description;
        void *storage;
        cv_internal::CVType type;
    };

    std::map<std::string, Entry> entries_;

public:
    template<typename T>
    void setupVar(const std::string& name, const char *description, T *storage)
    {
        entries_.emplace(name, Entry {
            description, storage, cv_internal::getType<T>()
        });
    }

    void load();
    void save();
};
