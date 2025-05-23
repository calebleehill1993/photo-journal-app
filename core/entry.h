#ifndef ENTRY_H
#define ENTRY_H

#include <string>
#include <nlohmann/json.hpp>
#include "../utils/time_utils.h"
#include "../config/config_handler.h"
#include <regex>

class Entry {
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
    
    std::string to_filename() {
        return generatedId + ".png";
    }
    
    std::string to_timestamp() {
        return date + "T" + time + timeOffset;
    }

    std::string get_exif_datetime() {
        std::string exif_datetime = date + " " + time;
        std::replace(exif_datetime.begin(), exif_datetime.end(), '-', ':');
        return exif_datetime;
    }
    
    std::string formatted_id(const std::string& id) {
        std::string formatted = id;

        // Convert to lowercase
        transform(formatted.begin(), formatted.end(), formatted.begin(), ::tolower);
        
        // For the timestamp formatting
        replace(formatted.begin(), formatted.end(), ':', '-');

        // Replace illegal characters (anything that is not alphanumeric, _, +, or -) with '-'
        formatted = std::regex_replace(formatted, std::regex("[^a-z0-9_+-]"), "_");

        return formatted;
    }
    
    std::string getFirstNCharsFromVector(const std::vector<std::string>& input, int chars_needed) {
        // Concatenate characters from the vector until we have the first 10 characters
        std::string truncated_paragraphs;

        for (const std::string& str : input) {
            if (chars_needed > 0) {
                int chars_to_take = std::min(chars_needed, (int)str.length());
                truncated_paragraphs += str.substr(0, chars_to_take);
                chars_needed -= chars_to_take;
            }
            if (chars_needed == 0) {
                break;
            }
        }
        
        return truncated_paragraphs;
    }
    
    std::string generateId() {
        std::string id;
        id += to_timestamp() + "_";
        for (std::string tag : tags) {
            id += tag + "_";
        }
        id += entryType + "_";
        id += getFirstNCharsFromVector(title, 10) + "_";
        id += getFirstNCharsFromVector(body, 10);
        
        return formatted_id(id);
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
    
    std::string to_string() {
        nlohmann::json json_entry;
        json_entry["date"] = date;
        json_entry["time"] = time;
        json_entry["timeOffset"] = timeOffset;
        json_entry["tags"] = tags;
        json_entry["title"] = title;
        json_entry["body"] = body;
        json_entry["entryType"] = entryType;
        json_entry["generatedId"] = generatedId;
        
        return json_entry.dump(2);
    }

    std::vector<std::string> to_vector() {
        std::vector<std::string> vector_of_strings = {
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

        return vector_of_strings;
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
    
};

#endif