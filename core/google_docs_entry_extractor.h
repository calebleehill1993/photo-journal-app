#ifndef GOOGLE_DOCS_ENTRY_EXTRACTOR_H
#define GOOGLE_DOCS_ENTRY_EXTRACTOR_H

#include <string>
#include <regex>
#include <nlohmann/json.hpp>
#include "entry.h"
#include "entry_extractor.h"
#include "../utils/time_utils.h"
#include "../utils/string_utils.h"
#include "../config/config_handler.h"

class GoogleDocsEntryExtractor : public EntryExtractor {

public:

    GoogleDocsEntryExtractor(const std::string& docJson)
        : docJson(docJson) {
        defaultTime = ConfigHandler::getInstance().getConfigValue("settings", "default_time");
        defaultTimezoneOffset = ConfigHandler::getInstance().getConfigValue("settings", "default_timezone_offset");
        adjustForDaylightSavings = ConfigHandler::getInstance().getConfigValue("settings", "adjust_for_daylight_savings");
        entryType = "Google Docs";
    }
    ~GoogleDocsEntryExtractor() override = default;

    std::vector<Entry> extractEntries() override {
        nlohmann::json data = nlohmann::json::parse(docJson);

        std::vector<Entry> entries;

        nlohmann::json content = data["body"]["content"];

        std::string currentDate;
        std::string currentTime;
        std::string currentTimeOffset;
        std::vector<std::string> currentTags;
        std::vector<std::string> currentTitle;
        std::vector<std::string> currentBody;
        int newLineCounter = 0;

        std::regex datePattern(R"(\d{4}-\d{2}-\d{2})");
        std::regex timePattern(R"((\d{2}:\d{2}(:\d{2})?)([+-]\d{2}:\d{2})?)");

        for (const auto& element : content) {
            if (element.contains("paragraph")) {
                auto textElements = element["paragraph"]["elements"];
                std::string text;
                bool isBold = false;

                // Concatenate all the text content
                for (const auto& e : textElements) {
                    if (e.contains("textRun")) {
                        const auto& textRun = e["textRun"];
                        std::string content = textRun["content"].get<std::string>();
                        text += content;

                        if (isBold == false && content != "\n" && textRun["textStyle"].contains("bold")) {
                            isBold = textRun["textStyle"]["bold"].get<bool>();
                        }
                    }
                }

                text = std::regex_replace(text, std::regex("^\\s+|\\s+$"), ""); // Trim spaces

                if (text.empty()) {
                    newLineCounter++;

                    if (newLineCounter == 2) {
                        if (currentTime.length() == 0) {
                            currentTime = defaultTime;
                            currentTimeOffset = adjustedDefaultTimeOffset(currentDate, currentTime, defaultTimezoneOffset, adjustForDaylightSavings);
                        }

                        entries.push_back(Entry(currentDate, currentTime, currentTimeOffset, currentTags, currentTitle, currentBody, entryType));
                        currentTime = "";
                        currentTimeOffset = "";
                        currentTags.clear();
                        currentTitle.clear();
                        currentBody.clear();
                    }

                    continue;
                }

                if (regex_match(text, datePattern)) {
                    currentDate = text;
                } else if (currentTags.empty() && currentTime.length() == 0 && (regex_match(text, timePattern))) {
                    TimeUtils::TimeInfo timeInfo = TimeUtils::extractTimeInfo(text, timePattern);
                    currentTime = timeInfo.time;

                    if (!timeInfo.timeOffset.empty()) {
                        currentTimeOffset = timeInfo.timeOffset;
                    } else {
                        currentTimeOffset = adjustedDefaultTimeOffset(currentDate, currentTime, defaultTimezoneOffset, adjustForDaylightSavings);
                    }

                } else if (currentTags.empty() && StringUtils::isAllUppercaseLetters(text)) {
                    currentTags = StringUtils::split(text, '|');
                } else {
                    if (newLineCounter == 0) {
                        if (isBold) {
                            if (currentTitle.empty()) {
                                currentTitle.push_back(text);
                            } else {
                                currentTitle.back() += "\n" + text;
                            }
                        } else {
                            if (currentBody.empty()) {
                                currentBody.push_back(text);
                            } else {
                                currentBody.back() += "\n" + text;
                            }
                        }
                    } else {
                        if (isBold) {
                            currentTitle.push_back(text);
                        } else {
                            currentBody.push_back(text);
                        }
                    }
                }

                newLineCounter = 0;
            }
        }

        if (currentTime.length() == 0) {
            currentTime = defaultTime;
            currentTimeOffset = adjustedDefaultTimeOffset(currentDate, currentTime, defaultTimezoneOffset, adjustForDaylightSavings);
        }
        entries.push_back(Entry(currentDate, currentTime, currentTimeOffset, currentTags, currentTitle, currentBody, entryType));

        return entries;
    }

private:
    std::string docJson;
    std::string defaultTime;
    std::string defaultTimezoneOffset;
    bool adjustForDaylightSavings;
    std::string entryType;

    std::string adjustedDefaultTimeOffset(const std::string& currentDate, const std::string& currentTime, const std::string& defaultTimezoneOffset, const bool& adjustForDaylightSavings) {
        if (adjustForDaylightSavings) {
            return TimeUtils::adjustForDaylightSavings(currentDate, currentTime, defaultTimezoneOffset);
        } else {
            return defaultTimezoneOffset;
        }
    }

    std::vector<std::string> getLines() {
        nlohmann::json data = nlohmann::json::parse(docJson);
        std::vector<std::string> lines;
        nlohmann::json content = data["body"]["content"];

        for (const auto& element : content) {
            if (element.contains("paragraph")) {
                auto textElements = element["paragraph"]["elements"];
                std::string text;
                bool isBold = false;

                // Concatenate all the text content
                for (const auto& e : textElements) {
                    if (e.contains("textRun")) {
                        const auto& textRun = e["textRun"];
                        std::string content = textRun["content"].get<std::string>();
                        text += content;

                        if (isBold == false && content != "\n" && textRun["textStyle"].contains("bold")) {
                            isBold = textRun["textStyle"]["bold"].get<bool>();
                        }
                    }
                }

                if (isBold) {
                    text = "**" + text;
                }

                lines.push_back(text);
            }
        } 
        return lines;
    }

};

#endif
