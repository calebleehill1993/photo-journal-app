#include "entry.h"
#include <iostream>
#include <sstream>
#include <regex>
#include <nlohmann/json.hpp>
#include "../utils/time_utils.h"
#include "../config/config_handler.h"

namespace ConfigConst = ConfigConstants;

Entry::Entry(std::string date, std::string time, std::string timeOffset, std::vector<std::string> tags, std::vector<std::string> title, std::vector<std::string> body, std::string entryType) {
    this->date = date;
    this->time = time;
    this->timeOffset = timeOffset;
    this->tags = tags;
    this->title = title;
    this->body = body;
    this->entryType = entryType;
    this->generatedId = generateId();
}

void Entry::setPhotosId(std::string& photosId) {
    this->photosId = photosId;
}

std::string Entry::getPhotosId() {
    return photosId;
}

std::string Entry::getDate() {
    return date;
}

std::string Entry::getTime() {
    return time;
}

std::string Entry::getTimeOffset() {
    return timeOffset;
}

std::vector<std::string> Entry::getTags() {
    return tags;
}

std::vector<std::string> Entry::getTitle() {
    return title;
}

std::vector<std::string> Entry::getBody() {
    return body;
}

std::string Entry::getEntryType() {
    return entryType;
}

std::string Entry::toFilename() {
    return generatedId + ".png";
}

std::string Entry::toTimestamp() {
    return date + "T" + time + timeOffset;
}

std::string Entry::getExifDatetime() {
    std::string exifDatetime = date + " " + time;
    std::replace(exifDatetime.begin(), exifDatetime.end(), '-', ':');
    return exifDatetime;
}

std::string Entry::getFirstNCharsFromParagraphs(const std::vector<std::string>& paragraphs, int charsNeeded) {
    std::string firstNChars;
    for (const std::string& paragraph : paragraphs) {
        if (charsNeeded > 0) {
            int charsToTake = std::min(charsNeeded, (int)paragraph.length());
            firstNChars += paragraph.substr(0, charsToTake);
            charsNeeded -= charsToTake;
        }
        if (charsNeeded == 0) {
            break;
        }
    }
    
    return firstNChars;
}

std::string Entry::generateId() {
    std::string id;
    id += toTimestamp() + "_";
    for (std::string tag : tags) {
        id += tag + "_";
    }
    id += entryType + "_";

    const int TITLE_CHARS = 10;
    const int BODY_CHARS = 10;
    id += getFirstNCharsFromParagraphs(title, TITLE_CHARS) + "_";
    id += getFirstNCharsFromParagraphs(body, BODY_CHARS);
    
    return formattedId(id);
}

std::string Entry::generatePhotosDescription() {
    std::string description = "";
    std::string ellipse = "...";

    int photosDescriptionCharLimit = ConfigHandler::getInstance().getConfigValue(ConfigConst::SETTINGS, ConfigConst::PHOTOS_DESCRIPTION_CHAR_LIMIT);
    int maxDescriptionLength = photosDescriptionCharLimit - ellipse.length();
    
    if (entryType.length() > 0) {
        description = description + "Type: " + entryType + "\n\n";
    }
    
    description = description + getBodyString();

    if (description.length() > photosDescriptionCharLimit) {
        description = description.substr(0, maxDescriptionLength) + ellipse;
    }
    
    return description;
}

std::string Entry::toString() {
    nlohmann::json jsonEntry;
    jsonEntry["date"] = date;
    jsonEntry["time"] = time;
    jsonEntry["timeOffset"] = timeOffset;
    jsonEntry["tags"] = tags;
    jsonEntry["title"] = title;
    jsonEntry["body"] = body;
    jsonEntry["entryType"] = entryType;
    jsonEntry["generatedId"] = generatedId;
    
    const int JSON_INDENT = 2;
    return jsonEntry.dump(JSON_INDENT);
}

std::vector<std::string> Entry::toVector() {
    std::vector<std::string> vectorOfStrings = {
        date,
        time,
        timeOffset,
        getTagsString(),
        getTitleString(),
        getBodyString(),
        entryType,
        generatedId,
        photosId,
        TimeUtils::computeUtcDateTime(date, time, timeOffset)
    };

    return vectorOfStrings;
}

std::string Entry::getTitleString(const std::string& delimiter) {
    return vectorToString(title, delimiter);
}

std::string Entry::getBodyString(const std::string& delimiter) {
    return vectorToString(body, delimiter);
}

std::string Entry::getTagsString(const std::string& delimiter) {
    return vectorToString(tags, delimiter);
}

std::string Entry::formattedId(const std::string& id) {
    std::string formatted = id;

    transform(formatted.begin(), formatted.end(), formatted.begin(), ::tolower);
    replace(formatted.begin(), formatted.end(), ':', '-');
    formatted = std::regex_replace(formatted, std::regex("[^a-z0-9_+-]"), "_");

    return formatted;
}

std::string Entry::vectorToString(const std::vector<std::string>& vec, const std::string& delimiter) {
    std::ostringstream oss;
    for (size_t i = 0; i < vec.size(); ++i) {
        oss << vec[i];
        if (i != vec.size() - 1) {
            oss << delimiter;
        }
    }
    return oss.str();
}
