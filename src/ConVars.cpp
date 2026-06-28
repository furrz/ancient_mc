#include "ConVars.h"

#include "read_json.h"

void ConVars::loadFile(const char *path)
{
    const auto data = read_json(path);

    for (const auto& [key, value] : data.items()) {
        const auto entry = entries_.find(key);
        if (entry == entries_.end()) {
            std::cerr << "res/config.json has invalid convar: " << key << std::endl;
            continue;
        }

        switch (entry->second.type) {
        case cv_internal::CVType::Float:
            *static_cast<float *>(entry->second.storage) = value["value"];
            break;
        case cv_internal::CVType::Int:
            *static_cast<int *>(entry->second.storage) = value["value"];
            break;
        case cv_internal::CVType::Bool:
            *static_cast<bool *>(entry->second.storage) = value["value"];
            break;
        }
    }
}

void ConVars::saveFile(const char *path, const std::vector<std::string>& keys)
{
    auto output = nlohmann::ordered_json::object();
    for (const auto& name : keys) {
        const auto [description, storage, type] = entries_.at(name);

        nlohmann::json value = nullptr;
        switch (type) {
        case cv_internal::CVType::Float:
            value = *static_cast<float*>(storage);
            break;
        case cv_internal::CVType::Int:
            value = *static_cast<int*>(storage);
            break;
        case cv_internal::CVType::Bool:
            value = *static_cast<bool*>(storage);
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
