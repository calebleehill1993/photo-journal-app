#include <stdexcept>

// Get a configuration value by traversing keys (only string keys are valid)
template <typename... Keys>
nlohmann::json ConfigHandler::getConfigValue(Keys&&... keys) const {
    const nlohmann::json* current = &configData;

    auto traverse = [&](const std::string& key) {
        if (current->contains(key)) {
            current = &(*current)[key];
        } else {
            throw std::runtime_error("Key not found in config: " + key);
        }
    };

    (traverse(std::forward<Keys>(keys)), ...);

    return *current;
}

// The last key is actually the value to be set
template <typename... Keys>
void ConfigHandler::setConfigValue(Keys&&... keys) {
    constexpr size_t numKeys = sizeof...(keys);
    if (numKeys < 2) {
        throw std::invalid_argument("At least two arguments are required: a key and a value.");
    }

    nlohmann::json* current = &configData;

    auto traverse = [&](const std::string& key) {
        if (current->contains(key)) {
            current = &(*current)[key];
        } else {
            // Create a new object if the key does not exist
            (*current)[key] = nlohmann::json::object();
            current = &(*current)[key];
        }
    };

    // Traverse the JSON object using the keys, except the last two
    auto keyList = {std::forward<Keys>(keys)...};
    for (auto itor = keyList.begin(); itor != std::prev(keyList.end(), 2); ++itor) {
        traverse(*itor);
    }

    // Set the second to last key to the last value
    (*current)[*std::prev(keyList.end(), 2)] = *std::prev(keyList.end());

    writeJson();
}