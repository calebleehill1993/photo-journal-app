#ifndef ENTRY_H
#define ENTRY_H

#include <string>
#include <regex>
#include <nlohmann/json.hpp>
#include "../utils/time_utils.h"
#include "../config/config_handler.h"

class Entry {
    
public:
    Entry(std::string date, std::string time, std::string timeOffset, std::vector<std::string> tags, std::vector<std::string> title, std::vector<std::string> body, std::string entryType) {
        this->date = date;
        this->time = time;
        this->timeOffset = timeOffset;
        this->tags = tags;
        this->title = title;
        this->body = body;
        this->entryType = entryType;
        this->generatedId = generateId();
    }

    void setPhotosId(std::string& photosId) {
        std::cout << "setting PhotosID";
        this->photosId = photosId;
    }

    std::string getPhotosId() {
        return photosId;
    }
    
    std::string getDate() {
        return date;
    }
    
    std::string getTime() {
        return time;
    }
    
    std::string getTimeOffset() {
        return timeOffset;
    }
    
    std::vector<std::string> getTags() {
        return tags;
    }
    
    std::vector<std::string> getTitle() {
        return title;
    }
    
    std::vector<std::string> getBody() {
        return body;
    }
    
    std::string getEntryType() {
        return entryType;
    }
    
    std::string toFilename() {
        return generatedId + ".png";
    }
    
    std::string toTimestamp() {
        return date + "T" + time + timeOffset;
    }

    std::string getExifDatetime() {
        std::string exifDatetime = date + " " + time;
        std::replace(exifDatetime.begin(), exifDatetime.end(), '-', ':');
        return exifDatetime;
    }
    
    std::string getFirstNCharsFromVector(const std::vector<std::string>& input, int charsNeeded) {
        std::string truncatedParagraphs;

        for (const std::string& str : input) {
            if (charsNeeded > 0) {
                int charsToTake = std::min(charsNeeded, (int)str.length());
                truncatedParagraphs += str.substr(0, charsToTake);
                charsNeeded -= charsToTake;
            }
            if (charsNeeded == 0) {
                break;
            }
        }
        
        return truncatedParagraphs;
    }
    
    std::string generateId() {
        std::string id;
        id += toTimestamp() + "_";
        for (std::string tag : tags) {
            id += tag + "_";
        }
        id += entryType + "_";
        id += getFirstNCharsFromVector(title, 10) + "_";
        id += getFirstNCharsFromVector(body, 10);
        
        return formattedId(id);
    }

    std::string generatePhotosDescription() {
        std::string description = "";
        std::string ellipse = "...";
        int photosDescriptionCharLimit = ConfigHandler::getInstance().getConfigValue("settings", "photos_description_char_limit");
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
    
    std::string toString() {
        nlohmann::json jsonEntry;
        jsonEntry["date"] = date;
        jsonEntry["time"] = time;
        jsonEntry["timeOffset"] = timeOffset;
        jsonEntry["tags"] = tags;
        jsonEntry["title"] = title;
        jsonEntry["body"] = body;
        jsonEntry["entryType"] = entryType;
        jsonEntry["generatedId"] = generatedId;
        
        return jsonEntry.dump(2);
    }

    std::vector<std::string> toVector() {
        std::vector<std::string> vectorOfStrings = {
            date,
            time,
            timeOffset,
            this->getTagsString(),
            this->getTitleString(),
            this->getBodyString(),
            entryType,
            generatedId,
            photosId,
            TimeUtils::computeUtcDateTime(date, time, timeOffset)
        };

        return vectorOfStrings;
    }
    
    std::string getTitleString(const std::string& delimiter = "\n\n") {
        return vectorToString(title, delimiter);
    }
    
    std::string getBodyString(const std::string& delimiter = "\n\n") {
        return vectorToString(body, delimiter);
    }

    std::string getTagsString(const std::string& delimiter = "|") {
        return vectorToString(tags, delimiter);
    }

private:
    std::string date;
    std::string time;
    std::string timeOffset;
    std::vector<std::string> tags;
    std::vector<std::string> title;
    std::vector<std::string> body;
    std::string entryType;
    std::string generatedId;
    std::string photosId;

    std::string formattedId(const std::string& id) {
        std::string formatted = id;

        transform(formatted.begin(), formatted.end(), formatted.begin(), ::tolower);
        replace(formatted.begin(), formatted.end(), ':', '-');
        formatted = std::regex_replace(formatted, std::regex("[^a-z0-9_+-]"), "_");

        return formatted;
    }

    std::string vectorToString(const std::vector<std::string>& vec, const std::string& delimiter = "|") {
        std::ostringstream oss;
        for (size_t i = 0; i < vec.size(); ++i) {
            oss << vec[i];
            if (i != vec.size() - 1) {
                oss << delimiter;
            }
        }
        return oss.str();
    }
    
};

#endif
