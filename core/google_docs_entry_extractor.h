#ifndef GOOGLE_DOCS_ENTRY_EXTRACTOR_H
#define GOOGLE_DOCS_ENTRY_EXTRACTOR_H

#include <string>
#include <regex>
#include <nlohmann/json.hpp>
#include "entry.h"
#include "entry_extractor.h"
#include "../utils/time_utils.h"

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> result;
    std::string token;
    std::istringstream token_stream(str);

    while (getline(token_stream, token, delimiter)) {
        result.push_back(token);
    }

    return result;
}

// Helper function to check if all alphabetic characters in a string are uppercase
bool is_all_uppercase_letters(const std::string& text) {
    for (char c : text) {
        if (isalpha(c) && !isupper(c)) {
            return false;
        }
    }
    return true;
}

struct TimeInfo {
    std::string time;
    std::string timeOffset; // Use std::optional to handle missing offset
};

/**
 * Extracts the time and optional time offset from a given string.
 */
TimeInfo extract_time_info(const std::string& input, std::regex& time_pattern) {
    std::smatch match;
    std::string time;
    if (regex_search(input, match, time_pattern)) {
        if (!match[2].str().empty()) {
            time = match[1].str();
        }
        
        else {
            time = match[1].str() + ":00";
        }

        // Extract HH:MM or HH:MM:SS
        std::string offset = match[3].str(); // Extract +HH:MM or -HH:MM (if present)

        return {time, offset};
    }

    // Return empty if no match found
    return {"", ""};
}

std::string adjusted_default_time_offset(const std::string& current_date, const std::string& current_time, const std::string& defaultTimezoneOffset, const bool& adjustForDaylightSavings) {
    if (adjustForDaylightSavings) {
        return TimeUtils::adjust_for_daylight_savings(current_date, current_time, defaultTimezoneOffset);
    } else {
        return defaultTimezoneOffset;
    }
}

class GoogleDocsEntryExtractor : public EntryExtractor {
public:

    GoogleDocsEntryExtractor(const std::string& docJson)
        : docJson(docJson) {
        defaultTimezoneOffset = ConfigHandler::getInstance().getConfigValue("settings", "default_timezone_offset");
        adjustForDaylightSavings = ConfigHandler::getInstance().getConfigValue("settings", "adjust_for_daylight_savings");
    }
    ~GoogleDocsEntryExtractor() override = default;

    std::vector<Entry> extract_entries() override {
        nlohmann::json data = nlohmann::json::parse(docJson);

        std::vector<Entry> entries;
        nlohmann::json content = data["body"]["content"];

        std::string current_date;
        std::string current_time;
        std::string current_time_offset;
        std::vector<std::string> current_tags;
        std::vector<std::string> current_title;
        std::vector<std::string> current_body;
        int new_line_counter = 0;

        std::regex date_pattern(R"(\d{4}-\d{2}-\d{2})");
        std::regex time_pattern(R"((\d{2}:\d{2}(:\d{2})?)([+-]\d{2}:\d{2})?)");

        for (const auto& element : content) {
            if (element.contains("paragraph")) {
                auto text_elements = element["paragraph"]["elements"];
                std::string text;
                bool is_bold = false;

                // Concatenate all the text content
                for (const auto& e : text_elements) {
                    if (e.contains("textRun")) {
                        const auto& text_run = e["textRun"];
                        std::string content = text_run["content"].get<std::string>();
                        text += content;

                        if (is_bold == false && content != "\n" && text_run["textStyle"].contains("bold")) {
                            is_bold = text_run["textStyle"]["bold"].get<bool>();
                        }
                    }
                }

                text = std::regex_replace(text, std::regex("^\\s+|\\s+$"), ""); // Trim spaces

                if (text.empty()) {
                    new_line_counter++;

                    if (new_line_counter == 2) {
                        if (current_time.length() == 0) {
                            current_time = "23:59:59"; // DEFAULT TIME
                            current_time_offset = adjusted_default_time_offset(current_date, current_time, defaultTimezoneOffset, adjustForDaylightSavings);
                        }

                        entries.push_back(Entry(current_date, current_time, current_time_offset, current_tags, current_title, current_body, "Google Docs"));
                        current_time = "";
                        current_time_offset = "";
                        current_tags.clear();
                        current_title.clear();
                        current_body.clear();
                    }

                    continue;
                }

                if (regex_match(text, date_pattern)) {
                    current_date = text;
                } else if (current_tags.empty() && current_time.length() == 0 && (regex_match(text, time_pattern))) {
                    TimeInfo timeInfo = extract_time_info(text, time_pattern);
                    current_time = timeInfo.time;

                    if (!timeInfo.timeOffset.empty()) {
                        current_time_offset = timeInfo.timeOffset;
                    } else {
                        current_time_offset = adjusted_default_time_offset(current_date, current_time, defaultTimezoneOffset, adjustForDaylightSavings);
                    }

                } else if (current_tags.empty() && is_all_uppercase_letters(text)) {
                    current_tags = split(text, '|');
                } else {
                    if (is_bold) {
                        current_title.push_back(text);
                    } else {
                        current_body.push_back(text);
                    }
                }

                new_line_counter = 0;
            }
        }

        if (current_time.length() == 0) {
            current_time = "23:59:59"; // DEFAULT TIME
            current_time_offset = adjusted_default_time_offset(current_date, current_time, defaultTimezoneOffset, adjustForDaylightSavings);
        }
        entries.push_back(Entry(current_date, current_time, current_time_offset, current_tags, current_title, current_body, "Google Docs"));

        return entries;
    }

private:
    std::string docJson;
    std::string defaultTimezoneOffset;
    bool adjustForDaylightSavings;

};

#endif
