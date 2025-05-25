#ifndef CONFIG_HANDLER_H
#define CONFIG_HANDLER_H

#include <string>
#include <nlohmann/json.hpp>

class ConfigHandler {
public:
    static ConfigHandler& getInstance();
    void loadConfig(const std::string& filePath);

    // Get a configuration value by traversing keys (only string keys are valid)
    template <typename... Keys>
    nlohmann::json getConfigValue(Keys&&... keys) const;

    // The last key is actually the value to be set
    template <typename... Keys>
    void setConfigValue(Keys&&... keys);

    void writeJson();

private:
    ConfigHandler();
    ConfigHandler(const ConfigHandler&) = delete;
    ConfigHandler& operator=(const ConfigHandler&) = delete;

private:
    nlohmann::json configData;

};

#include "config_handler.tpp"
#include "config_constants.h"

#endif // CONFIG_HANDLER_H
