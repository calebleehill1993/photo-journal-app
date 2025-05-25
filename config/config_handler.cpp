#include "config_handler.h"
#include <fstream>
#include <stdexcept>
#include <iostream>

namespace ConfigConst = ConfigConstants;

ConfigHandler& ConfigHandler::getInstance() {
    static ConfigHandler instance;
    return instance;
}

void ConfigHandler::loadConfig(const std::string& filePath) {
    std::cout << "Loading configuration from file: " << filePath << std::endl;

    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open config file: " << filePath << std::endl;
        throw std::runtime_error("Unable to open config file: " + filePath);
    }

    nlohmann::json jsonConfig;
    file >> jsonConfig;
    configData = jsonConfig;

    std::cout << "Configuration loading completed successfully." << std::endl;
}

void ConfigHandler::writeJson() {
    const int JSON_INDENT = 4;
    std::ofstream outputFile(ConfigConst::CONFIG_FILE_PATH);
    if (outputFile.is_open()) {
        outputFile << std::setw(JSON_INDENT) << configData << std::endl;
        outputFile.close();
        std::cout << "JSON data written to config.json" << std::endl;
    } else {
        std::cerr << "Error opening file for writing" << std::endl;
    }
}

ConfigHandler::ConfigHandler() {
    loadConfig(ConfigConst::CONFIG_FILE_PATH);
}

