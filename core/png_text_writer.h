#ifndef PNG_TEXT_WRITER_H
#define PNG_TEXT_WRITER_H

#include <pngwriter.h>
#include <cmath>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include "../config/config_handler.h"
#include <ctime>
#include <cstdlib>
#include <filesystem>
#include <typeinfo>
#include <algorithm>  // For std::transform, std::replace
#include <cctype>     // For ::tolower
#include <regex>      // For regex_replace

std::vector<std::string> split_sentence(std::string sen) {
  
    // Create a stringstream object
    std::stringstream ss(sen);
    
    // Variable to hold each word
    std::string word;
    
    // Vector to store the words
    std::vector<std::string> words;
    
    // Extract words from the sentence
    while (ss >> word) {
      
        // Add the word to the vector
        words.push_back(word);
    }
    
    return words;
}

bool string_contains_element(std::string str, const char* elements) {
    for (int i = 0; i < strlen(elements); i++) {
        if (str.find(elements[i]) != std::string::npos) {
            return true;
        }
    }
    return false;
}

std::string get_current_datetime() {
    // Get the current time and date
    time_t now = time(0);
    tm *ltm = localtime(&now);

    // Format the date and time as "YYYY-MM-DD HH:MM:SS"
    std::stringstream datetime;
    datetime << 1900 + ltm->tm_year << "-"
             << std::setw(2) << std::setfill('0') << 1 + ltm->tm_mon << "-"
             << std::setw(2) << std::setfill('0') << ltm->tm_mday << " "
             << std::setw(2) << std::setfill('0') << ltm->tm_hour << ":"
             << std::setw(2) << std::setfill('0') << ltm->tm_min << ":"
             << std::setw(2) << std::setfill('0') << ltm->tm_sec;
    
    return datetime.str();
};

class PngTextWriter {
    private:
        int aspect_ratio_width;
        int aspect_ratio_height;
        float aspect_ratio;
        float margins;
        int font_size;
        const char * descenders;
        int descender_spacing;
        int spacing;
        int paragraph_spacing;
        int width;
        int height;
        int left_margin;
        int right_margin;
        int bottom_margin;
        int top_margin;
        int horizontal_margin_size;
        int vertical_margin_size;
        int start_height;
        int line_size;
        int space_width;
        std::string line;
        int running_width;
        int line_number;
        int cursor;
        pngwriter image;
        std::vector<std::string> paragraphs;
        std::string timestamp;
        std::string filename;
        char* font;
    
    public:
    PngTextWriter(std::vector<std::string> paragraphs, std::string date, std::string time, std::string filename) {
        aspect_ratio_width = 7;
        aspect_ratio_height = 9;
        aspect_ratio = (float) aspect_ratio_width / aspect_ratio_height;
        margins = 0.05;
        std::string fontPath = ConfigHandler::getInstance().getConfigValue("settings", "font_path");
        font = new char[fontPath.length() + 1];
        strcpy(font, fontPath.c_str());
        font_size = 100;
        descenders = "gjpqy";
        descender_spacing = 30;
        spacing = 10;
        paragraph_spacing = 30;
        height = 1000;
        width = round(height * aspect_ratio);
//        image.filledsquare(1, 1, width, height, 0.2, 0.2, 0.2);
        left_margin = round(width * margins);
        right_margin = round(width * (1.0 - margins));
        bottom_margin = round(height * margins);
        top_margin = round(height * (1.0 - margins));
        horizontal_margin_size = right_margin - left_margin;
        vertical_margin_size = top_margin - bottom_margin;
        start_height = top_margin - font_size;
        space_width = image.get_text_width_utf8(font, font_size, " ");
        std::string line = "";
        running_width = 0;
        line_number = 0;
        cursor = top_margin - font_size;
        this->paragraphs = paragraphs;
        this->timestamp = date + "T" + time + "+07:00";
        this->filename = filename;
    }

    ~PngTextWriter() {
        delete[] font;
    }
    
    void set_dimensions(int new_height) {
        height = new_height;
        width = round(height * aspect_ratio);
        left_margin = round(width * margins);
        right_margin = round(width * (1.0 - margins));
        bottom_margin = round(height * margins);
        top_margin = round(height * (1.0 - margins));
        horizontal_margin_size = right_margin - left_margin;
        vertical_margin_size = top_margin - bottom_margin;
        image.resize(width, height);
    };
    
    int fit_image() {
        int text_height = font_size + descender_spacing;
        int paragraph_count = paragraphs.size();
        for (int j = 0; j < paragraph_count; j++) {
            
            std::vector<std::string> words = split_sentence(paragraphs.at(j));
            line = "";
            running_width = 0;

            if (j > 0) {
                text_height += font_size + descender_spacing + spacing;
            }
            
            int word_count = words.size();
            for (int i = 0; i < word_count; i++) {
                if (line.length() > 0) {
                    running_width += space_width;
                }

                char * word = new char[words.at(i).length() + 1];
                strcpy(word, words.at(i).c_str());
                
                int word_width = image.get_text_width_utf8(font, font_size, word);
                
                if (word_width > horizontal_margin_size) {
                    delete[] word;
                    set_dimensions(height + 100);
                    text_height = fit_image();
                    return text_height;
                }
                
                running_width += word_width;
                
                if (running_width <= horizontal_margin_size) {
                    if (line.length() > 0) {
                        line += " ";
                    }
                    
                    line += words.at(i);
                    if (i + 1 == word_count && j + 1 < paragraph_count) {
                        text_height += font_size + descender_spacing + spacing;
                    }
                }
                else {
                    running_width -= (space_width + word_width);
                    
                    text_height += font_size + descender_spacing + spacing;
                    
                    line = "";
                    i -= 1;
                    running_width = 0;
                }
                
                delete[] word;
            }
        }
        
        if (text_height > vertical_margin_size) {
            set_dimensions(height + 100);
            text_height = fit_image();
        }
        
        return text_height;
        
    };
    
    void write_line(bool create_lines = false) {
        bool line_has_descenders = string_contains_element(line, descenders);
        
        char * linecopy = new char[line.length() + 1];
        strcpy(linecopy, line.c_str());
        
        int line_horizontal_start = left_margin + round((horizontal_margin_size - running_width) / 2);
        
        if (create_lines) {
            int yfrom = cursor - (line_has_descenders * descender_spacing);
            int yto = cursor + font_size;
            image.square(line_horizontal_start, yfrom, line_horizontal_start + running_width, yto, 1.0, 0.0, 0.0);
        }
        
        image.plot_text_utf8(font, font_size, line_horizontal_start, cursor, 0.0, linecopy, 1.0, 1.0, 1.0);
        delete[] linecopy;
        
        cursor -= font_size + spacing + ((line_has_descenders || true) * descender_spacing);
    };
    
    void write_text() {
        
        image = pngwriter(width, height, 0.0, filename.c_str());
        bool show_line_boarders = false;
        int text_height = fit_image();
        cursor = top_margin - round((vertical_margin_size - text_height) / 2) - font_size;
        
        int paragraph_count = paragraphs.size();
        for (int j = 0; j < paragraph_count; j++) {
            std::vector<std::string> words = split_sentence(paragraphs.at(j));
            line = "";
            running_width = 0;
            
            if (j > 0) {
                cursor -= font_size + spacing + descender_spacing;
            }
            
            int word_count = words.size();
            for (int i = 0; i < word_count; i++) {
                if (line.length() > 0) {
                    running_width += space_width;
                }

                char * word = new char[words.at(i).length() + 1];
                strcpy(word, words.at(i).c_str());
                
                int word_width = image.get_text_width_utf8(font, font_size, word);
                
                running_width += word_width;
                
                if (running_width <= horizontal_margin_size) {
                    if (line.length() > 0) {
                        line += " ";
                    }
                    
                    line += words.at(i);
                    if (i + 1 == word_count) {
                        write_line(show_line_boarders);
                    }
                }
                else {
                    running_width -= (space_width + word_width);
                    
                    write_line(show_line_boarders);
                    
                    line_number += 1;
                    line = "";
                    i -= 1;
                    running_width = 0;
                }
                
                delete[] word;
            }
        }
        
        std::cout << "Successfully generated image." << std::endl;
        
        save_and_close();
        
        return;
    }
    
    void save_and_close() {
        image.close();
        std::cout << "Image successfully saved and closed." << std::endl;
    }
    
};

#endif