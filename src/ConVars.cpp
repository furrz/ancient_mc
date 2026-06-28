#include "ConVars.h"

#include <glm/detail/qualifier.hpp>

#include "read_json.h"

static void assignCVTypedVar(const cv_internal::CVType cvType, const nlohmann::json& value, void *storage)
{
    switch (cvType) {
    case cv_internal::CVType::Float:
        *static_cast<float *>(storage) = value;
        break;
    case cv_internal::CVType::Int:
        *static_cast<int *>(storage) = value;
        break;
    case cv_internal::CVType::Bool:
        *static_cast<bool *>(storage) = value;
        break;
    }
}


void ConVars::loadFile(const char *path)
{
    const auto data = read_json(path);

    for (const auto& [key, value]: data.items()) {
        const auto entry = entries_.find(key);
        if (entry == entries_.end()) {
            pendingEntries_.emplace(key, value["value"]);
            continue;
        }

        assignCVTypedVar(entry->second.type, value["value"], entry->second.storage);
    }
}

void ConVars::saveFile(const char *path, const std::vector<std::string>& keys)
{
    auto output = nlohmann::ordered_json::object();
    for (const auto& name: keys) {
        const auto [description, storage, type] = entries_.at(name);

        nlohmann::json value = nullptr;
        switch (type) {
        case cv_internal::CVType::Float:
            value = *static_cast<float *>(storage);
            break;
        case cv_internal::CVType::Int:
            value = *static_cast<int *>(storage);
            break;
        case cv_internal::CVType::Bool:
            value = *static_cast<bool *>(storage);
            break;
        }

        output[name] = nlohmann::ordered_json::object({
            { "description", description },
            { "value", value }
        });
    }

    std::ofstream out(path);
    out << std::setw(4) << output << std::endl;
    out.close();
}


void ConVars::setupVarGeneral(const std::string& name, const char *description, void *storage, const SaveType saveType,
                              const cv_internal::CVType cvType)
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

    entries_.emplace(
        name,
        Entry{
            description, storage, cvType
        });

    const auto found = pendingEntries_.find(name);
    if (found != pendingEntries_.end()) {
        assignCVTypedVar(cvType, found->second, storage);
        pendingEntries_.erase(found);
    }
}

void ConVars::load()
{
    loadFile("res/config.json");
    loadFile("settings.json");
}

void ConVars::save()
{
    saveFile("res/config.json", gameConfigKeys_);
    saveFile("settings.json", userPrefsKeys_);
}

void ConVars::validateNoPendingEntries()
{
    for (const auto& name: pendingEntries_ | std::views::keys) {
        std::cerr << "invalid convar: " << name << std::endl;
    }
}
