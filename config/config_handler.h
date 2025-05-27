#ifndef CONFIG_HANDLER_H
#define CONFIG_HANDLER_H

#include <string>
#include <nlohmann/json.hpp>

#include "config_constants.h"

class ConfigHandler {
public:
    static ConfigHandler& getInstance();

    // Get a configuration value by traversing keys (only string keys are valid)
    template <typename... Keys>
    nlohmann::json getConfigValue(Keys&&... keys) const;

    // The last key is actually the value to be set
    template <typename... Keys>
    void setConfigValue(Keys&&... keys);

private:
    ConfigHandler();
    ConfigHandler(const ConfigHandler&) = delete;
    ConfigHandler& operator=(const ConfigHandler&) = delete;
    void loadConfig(const std::string& filePath);
    void writeJson();

private:
    nlohmann::json configData;

};

#include "config_handler.tpp"

#endif // CONFIG_HANDLER_H
