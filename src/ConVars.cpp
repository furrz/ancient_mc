#include "ConVars.h"

#include "read_json.h"

void ConVars::load()
{
    const auto data = read_json("res/config.json");

    for (const auto& [key, value] : data.items()) {
        const auto entry = entries_.find(key.c_str());
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

void ConVars::save()
{
    nlohmann::ordered_json output{};
    for (const auto& [name, entry] : entries_) {
        nlohmann::json value = nullptr;
        switch (entry.type) {
        case cv_internal::CVType::Float:
            value = *static_cast<float*>(entry.storage);
            break;
        case cv_internal::CVType::Int:
            value = *static_cast<int*>(entry.storage);
            break;
        case cv_internal::CVType::Bool:
            value = *static_cast<bool*>(entry.storage);
            break;
        }

        output[name] = nlohmann::ordered_json::object({
            { "description", entry.description },
            { "value", value }
        });
    }

    std::ofstream out("res/config.json");
    out << std::setw(4) << output << std::endl;
    out.close();
}
