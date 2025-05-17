//
//  entry.h
//  google_photos_api_test
//
//  Created by Caleb Hill on 3/22/25.
//
#ifndef ENTRY_H
#define ENTRY_H

#include <string>
#include <nlohmann/json.hpp>
#include "dst.h"

using json = nlohmann::json;
using namespace std;

class Entry {
private:
    string date;
    string time;
    string timeOffset;
    vector<string> tags;
    vector<string> title;
    vector<string> body;
    string entryType;
    string generatedId;
    string photosId;
    
public:
    Entry(string date, string time, string timeOffset, vector<string> tags, vector<string> title, vector<string> body, string entryType) {
        this->date = date;
        this->time = time;
        this->timeOffset = timeOffset;
        this->tags = tags;
        this->title = title;
        this->body = body;
        this->entryType = entryType;
        this->generatedId = generateId();
    }

    string vectorToString(const std::vector<std::string>& vec, const std::string& delimiter = "|") {
        std::ostringstream oss;
        for (size_t i = 0; i < vec.size(); ++i) {
            oss << vec[i];
            if (i != vec.size() - 1) {
                oss << delimiter;
            }
        }
        return oss.str();
    }

    void setPhotosId(string& photosId) {
        cout << "setting PhotosID";
        this->photosId = photosId;
    }

    string getPhotosId() {
        return photosId;
    }
    
    string getDate() {
        return date;
    }
    
    string getTime() {
        return time;
    }
    
    string getTimeOffset() {
        return timeOffset;
    }
    
    vector<string> getTags() {
        return tags;
    }
    
    vector<string> getTitle() {
        return title;
    }

    string getTitleAsString(const std::string& delimiter = "|") {
        return vectorToString(title, delimiter);
    }
    
    vector<string> getBody() {
        return body;
    }

    string getBodyAsString(const std::string& delimiter = "|") {
        return vectorToString(body, delimiter);
    }
    
    string getEntryType() {
        return entryType;
    }
    
    string to_filename() {
        return generatedId + ".png";
    }
    
    string to_timestamp() {
        return date + "T" + time + timeOffset;
    }

    string get_exif_datetime() {
        string exif_datetime = date + " " + time;
        replace(exif_datetime.begin(), exif_datetime.end(), '-', ':');
        return exif_datetime;
    }
    
    string formatted_id(const string& id) {
        string formatted = id;

        // Convert to lowercase
        transform(formatted.begin(), formatted.end(), formatted.begin(), ::tolower);
        
        // For the timestamp formatting
        replace(formatted.begin(), formatted.end(), ':', '-');

        // Replace illegal characters (anything that is not alphanumeric, _, +, or -) with '-'
        formatted = regex_replace(formatted, regex("[^a-z0-9_+-]"), "_");

        return formatted;
    }
    
    string get_first_n_chars(vector<string> input, int chars_needed) {
        // Concatenate characters from the vector until we have the first 10 characters
        string truncated_paragraphs;

        for (const string& str : input) {
            if (chars_needed > 0) {
                int chars_to_take = min(chars_needed, (int)str.length());
                truncated_paragraphs += str.substr(0, chars_to_take);
                chars_needed -= chars_to_take;
            }
            if (chars_needed == 0) {
                break;
            }
        }
        
        return truncated_paragraphs;
    }
    
    string generateId() {
        string id;
        id += to_timestamp() + "_";
        for (string tag : tags) {
            id += tag + "_";
        }
        id += entryType + "_";
        id += get_first_n_chars(title, 10) + "_";
        id += get_first_n_chars(body, 10);
        
        return formatted_id(id);
    }
    
    string to_string() {
        json json_entry;
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

    vector<string> to_vector() {
        vector<string> vec_string = {
            date,
            time,
            timeOffset,
            this->getTagsString(),
            this->getTitleString(),
            this->getBodyString(),
            entryType,
            generatedId,
            photosId,
            computeUtcDateTime(date, time, timeOffset)
        };

        return vec_string;
    }
    
    string getTitleString(const std::string& delimiter = "\n\n") {
        return vectorToString(title, delimiter);
    }
    
    string getBodyString(const std::string& delimiter = "\n\n") {
        return vectorToString(body, delimiter);
    }

    string getTagsString(const std::string& delimiter = "|") {
        return vectorToString(tags, delimiter);
    }
    
};

#endif