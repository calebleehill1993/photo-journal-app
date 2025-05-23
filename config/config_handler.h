#ifndef CONFIG_HANDLER_H
#define CONFIG_HANDLER_H

#include <string>
#include <unordered_map>
#include <fstream>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <iostream>
#include <initializer_list>

class ConfigHandler {
public:
    // Get the singleton instance
    static ConfigHandler& getInstance() {
        static ConfigHandler instance;
        return instance;
    }

    // Load the configuration file
    void loadConfig(const std::string& filePath) {
        std::cout << "Loading configuration from file: " << filePath << std::endl;

        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Error: Unable to open config file: " << filePath << std::endl;
            throw std::runtime_error("Unable to open config file: " + filePath);
        }

        nlohmann::json jsonConfig;
        file >> jsonConfig;

        configData_ = jsonConfig;

        std::cout << "Configuration loading completed successfully." << std::endl;
    }

    // Get a configuration value by traversing keys (only string keys are valid)
    template <typename... Keys>
    nlohmann::json getConfigValue(Keys&&... keys) const {
        const nlohmann::json* current = &configData_;

        // Helper lambda to traverse the JSON object
        auto traverse = [&](const std::string& key) {
            if (current->contains(key)) {
                current = &(*current)[key];
            } else {
                throw std::runtime_error("Key not found in config: " + key);
            }
        };

        // Traverse the JSON object using the keys
        (traverse(std::forward<Keys>(keys)), ...);

        // Return the final JSON value
        return *current;
    }

    // The last key is actually the value to be set
    template <typename... Keys>
    void setConfigValue(Keys&&... keys) {
        constexpr size_t numKeys = sizeof...(keys);
        if (numKeys < 2) {
            throw std::invalid_argument("At least two arguments are required: a key and a value.");
        }

        nlohmann::json* current = &configData_;

        // Helper lambda to traverse the JSON object
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

    void writeJson() {
        // Write JSON to file
        std::ofstream outputFile(configFilePath_);
        if (outputFile.is_open()) {
            outputFile << std::setw(4) << configData_ << std::endl;
            outputFile.close();
            std::cout << "JSON data written to config.json" << std::endl;
        } else {
            std::cerr << "Error opening file for writing" << std::endl;
        }
    }


private:
    ConfigHandler() {
        configFilePath_ = "config/config.json";
        loadConfig("config/config.json");
    }
    ~ConfigHandler() = default;

    ConfigHandler(const ConfigHandler&) = delete;
    ConfigHandler& operator=(const ConfigHandler&) = delete;

    std::string configFilePath_;
    nlohmann::json configData_;
};

#endif // CONFIG_HANDLER_H
