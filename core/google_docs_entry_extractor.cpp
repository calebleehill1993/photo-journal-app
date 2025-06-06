#include "google_docs_entry_extractor.h"
#include <regex>
#include "../utils/time_utils.h"
#include "../utils/string_utils.h"
#include "../config/config_handler.h"

namespace ConfigConst = ConfigConstants;

const int NEW_ENTRY = 2; // Number of new lines to trigger a new entry
const std::string ENTRY_TYPE = "Google Docs";
const std::string DOC_BODY = "body";
const std::string DOC_CONTENT = "content";
const std::string DOC_PARAGRAPH = "paragraph";
const std::string DOC_ELEMENTS = "elements";
const std::string DOC_TEXT_RUN = "textRun";
const std::string DOC_TEXT_STYLE = "textStyle";
const std::string DOC_STYLE_BOLD = "bold";

GoogleDocsEntryExtractor::GoogleDocsEntryExtractor(const std::string& docJson)
    : docJson(docJson) {
    defaultTime = ConfigHandler::getInstance().getConfigValue(ConfigConst::SETTINGS, ConfigConst::DEFAULT_TIME);
    defaultTimezoneOffset = ConfigHandler::getInstance().getConfigValue(ConfigConst::SETTINGS, ConfigConst::DEFAULT_TIMEZONE_OFFSET);
    adjustForDaylightSavings = ConfigHandler::getInstance().getConfigValue(ConfigConst::SETTINGS, ConfigConst::ADJUST_FOR_DAYLIGHT_SAVINGS);
}

std::vector<Entry> GoogleDocsEntryExtractor::extractEntries() {
    std::vector<Entry> entries;
    int newLineCounter = 0;
    nlohmann::json data = nlohmann::json::parse(docJson);
    nlohmann::json content = data[DOC_BODY][DOC_CONTENT];
    for (const auto& element : content) {
        if (element.contains(DOC_PARAGRAPH)) {
            auto textElements = element[DOC_PARAGRAPH][DOC_ELEMENTS];
            bool isBold;
            std::string text = getTextFromElements(textElements, isBold);
            text = StringUtils::trimSpaces(text);

            if (text.empty()) {
                newLineCounter++;

                if (newLineCounter == NEW_ENTRY) {
                    if (currentTime.length() == 0) {
                        currentTime = defaultTime;
                        currentTimeOffset = adjustedDefaultTimeOffset(currentDate, currentTime);
                    }

                    entries.push_back(Entry(currentDate, currentTime, currentTimeOffset, currentTags, currentTitle, currentBody, ENTRY_TYPE));
                    resetForNewEntry();
                }

                continue;
            }

            if (regex_match(text, TimeUtils::DATE_PATTERN)) {
                currentDate = text;
            } else if (currentTags.empty() && currentTime.length() == 0 && (regex_match(text, TimeUtils::TIME_PATTERN))) {
                TimeUtils::TimeInfo timeInfo = TimeUtils::extractTimeInfo(text);
                currentTime = timeInfo.time;

                if (timeInfo.timeOffset.empty()) {
                    currentTimeOffset = adjustedDefaultTimeOffset(currentDate, currentTime);
                } else {
                    currentTimeOffset = timeInfo.timeOffset;
                }

            } else if (currentTags.empty() && StringUtils::isAllUppercaseLetters(text)) {
                currentTags = StringUtils::split(text, '|');
            } else {
                if (isBold) {
                    currentTitle.push_back(text);
                } else {
                    currentBody.push_back(text);
                }
            }

            newLineCounter = 0;
        }
    }

    if (currentTime.length() == 0) {
        currentTime = defaultTime;
        currentTimeOffset = adjustedDefaultTimeOffset(currentDate, currentTime);
    }
    entries.push_back(Entry(currentDate, currentTime, currentTimeOffset, currentTags, currentTitle, currentBody, ENTRY_TYPE));

    return entries;
}

std::string GoogleDocsEntryExtractor::getTextFromElements(const nlohmann::json& elements, bool& isBold) {
    std::string text;
    isBold = false;
    for (const auto& e : elements) {
        if (e.contains(DOC_TEXT_RUN)) {
            const auto& textRun = e[DOC_TEXT_RUN];
            std::string content = textRun[DOC_CONTENT].get<std::string>();
            text += content;

            if (isBold == false && content != "\n" && textRun[DOC_TEXT_STYLE].contains(DOC_STYLE_BOLD)) {
                isBold = textRun[DOC_TEXT_STYLE][DOC_STYLE_BOLD].get<bool>();
            }
        }
    }
    return text;
}

void GoogleDocsEntryExtractor::resetForNewEntry() {
    currentTime = "";
    currentTimeOffset = "";
    currentTags.clear();
    currentTitle.clear();
    currentBody.clear();
}

std::string GoogleDocsEntryExtractor::adjustedDefaultTimeOffset(const std::string& currentDate, const std::string& currentTime) {
    if (adjustForDaylightSavings) {
        return TimeUtils::adjustOffsetForDaylightSavings(currentDate, currentTime, defaultTimezoneOffset);
    } else {
        return defaultTimezoneOffset;
    }
}
