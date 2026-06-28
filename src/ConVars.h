#pragma once
#include <map>
#include <string>
#include <vector>

namespace cv_internal {
enum class CVType
{
    Float,
    Int,
    Bool
};

template<typename T>
constexpr CVType getType() = delete;

template<>
constexpr CVType getType<float>() { return CVType::Float; }

template<>
constexpr CVType getType<int>() { return CVType::Int; }

template<>
constexpr CVType getType<bool>() { return CVType::Bool; }
}


class ConVars
{
public:
    enum class SaveType
    {
        GameConfig,
        UserPrefs,
        None
    };

private:
    struct Entry
    {
        const char *description;
        void *storage;
        cv_internal::CVType type;
    };

    std::map<std::string, Entry> entries_;
    std::vector<std::string> gameConfigKeys_;
    std::vector<std::string> userPrefsKeys_;

    void loadFile(const char *path);
    void saveFile(const char *path, const std::vector<std::string>& keys);

public:
    template<typename T>
    void setupVar(const std::string& name, const char *description, T *storage, const SaveType saveType = SaveType::GameConfig)
    {
        switch (saveType) {
        case SaveType::GameConfig:
            gameConfigKeys_.push_back(name);
            break;
        case SaveType::UserPrefs:
            userPrefsKeys_.push_back(name);
            break;
        default: break;
        }

        entries_.emplace(name,
                         Entry{
                             description, storage, cv_internal::getType<T>()
                         });
    }

    void load();
    void save();
};
