#ifndef DOC_JSON_FORMATTER_H
#define DOC_JSON_FORMATTER_H

#include <string>
#include <regex>
#include <nlohmann/json.hpp>
#include "entry.h"
#include "dst.h"

using json = nlohmann::json;
using namespace std;

vector<string> split(const string& str, char delimiter) {
    vector<string> result;
    string token;
    istringstream token_stream(str);

    while (getline(token_stream, token, delimiter)) {
        result.push_back(token);
    }

    return result;
}


// Helper function to check if all alphabetic characters in a string are uppercase
bool is_all_uppercase_letters(const string& text) {
    for (char c : text) {
        if (isalpha(c) && !isupper(c)) {
            return false;
        }
    }
    return true;
}


struct TimeInfo {
    string time;
    string timeOffset; // Use std::optional to handle missing offset
};

/**
  * Extracts the time and optional time offset from a given string.
  */
TimeInfo extract_time_info(const string& input, regex& time_pattern) {
     // Regular expression to match various time formats
     
     smatch match;
     string time;
     if (regex_search(input, match, time_pattern)) {
         if (!match[2].str().empty()) {
             time = match[1].str();
         }
         else {
             time = match[1].str() + ":00";
         }
          // Extract HH:MM or HH:MM:SS
         string offset = match[3].str(); // Extract +HH:MM or -HH:MM (if present)

         return {time, offset};
     }

     // Return empty if no match found
     return {"", ""};
 }


string adjusted_default_time_offset(const string& current_date, const string& current_time, const string& defaultTimezoneOffset, const bool& adjustForDaylightSavings) {
    if (adjustForDaylightSavings) {
        return adjust_for_daylight_savings(current_date, current_time, defaultTimezoneOffset);
    }
    else {
        return defaultTimezoneOffset;
    }
}


vector<Entry> extract_entries(const string& originalJson, const string& defaultTimezoneOffset, const bool& adjustForDaylightSavings) {
    
    json data = json::parse(originalJson);

    vector<Entry> entries;
    json content = data["body"]["content"];
    
    string current_date;
    string current_time;
    string current_time_offset;
    vector<string> current_tags;
    vector<string> current_title;
    vector<string> current_body;
    int new_line_counter = 0;

    regex date_pattern(R"(\d{4}-\d{2}-\d{2})");
    regex time_pattern(R"((\d{2}:\d{2}(:\d{2})?)([+-]\d{2}:\d{2})?)");

    for (const auto& element : content) {
        if (element.contains("paragraph")) {
            auto text_elements = element["paragraph"]["elements"];
            string text;
            bool is_bold = false;

            // Concatenate all the text content
            for (const auto& e : text_elements) {
                if (e.contains("textRun")) {
                    const auto& text_run = e["textRun"];
                    text += text_run["content"].get<string>();
                    
                    if (text_run["textStyle"].contains("bold") && text_run["textStyle"]["bold"].get<bool>()) {
                        is_bold = true;
                    }
                }
            }

            text = regex_replace(text, regex("^\\s+|\\s+$"), ""); // Trim spaces

            if (text.empty()) {
                new_line_counter++;

                if (new_line_counter == 2) {
                    if (current_time.length() == 0) {
                        current_time = "23:59:59";  // DEFAULT TIME
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
            }
            
            else if (current_tags.empty() && current_time.length() == 0 && (regex_match(text, time_pattern))) {
                TimeInfo timeInfo = extract_time_info(text, time_pattern);
                current_time = timeInfo.time;
                
                cout << timeInfo.timeOffset << endl;
                cout << timeInfo.timeOffset.empty() << endl;
                
                if (!timeInfo.timeOffset.empty()) {
                    current_time_offset = timeInfo.timeOffset;
                }
                else {
                    current_time_offset = adjusted_default_time_offset(current_date, current_time, defaultTimezoneOffset, adjustForDaylightSavings);
                }
                
            }
            
            else if (current_tags.empty() && is_all_uppercase_letters(text)) {
                current_tags = split(text, '|');
            }
            
            else {
                if (is_bold) {
                    current_title.push_back(text);
                }
                
                else {
                    current_body.push_back(text);
                }
            }

            new_line_counter = 0;
        }
    }

    if (current_time.length() == 0) {
        current_time = "23:59:59";  // DEFAULT TIME
        current_time_offset = adjusted_default_time_offset(current_date, current_time, defaultTimezoneOffset, adjustForDaylightSavings);
    }
    entries.push_back(Entry(current_date, current_time, current_time_offset, current_tags, current_title, current_body, "Google Docs"));

    return entries;
}

#endif