#pragma once
#include <map>
#include <string>
#include <unordered_map>
#include <vector>
#include <nlohmann/json.hpp>

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
    std::map<std::string, nlohmann::json> pendingEntries_;

    void loadFile(const char *path);
    void saveFile(const char *path, const std::vector<std::string>& keys);
    void setupVarGeneral(const std::string& name, const char *description, void *storage, SaveType saveType, cv_internal::CVType cvType);

public:

    template<typename T>
    void setupVar(const std::string& name, const char *description, T *storage, const SaveType saveType = SaveType::GameConfig)
    {
        setupVarGeneral(name, description, storage, saveType, cv_internal::getType<T>());
    }

    void load();
    void save();
    void validateNoPendingEntries();
};
