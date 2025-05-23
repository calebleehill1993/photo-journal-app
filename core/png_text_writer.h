#ifndef PNG_TEXT_WRITER_H
#define PNG_TEXT_WRITER_H

#include <pngwriter.h>
#include <cmath>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <filesystem>
#include <typeinfo>
#include <algorithm>
#include <cctype>
#include <regex>
#include "../utils/string_utils.h"
#include "../config/config_handler.h"

class PngTextWriter {
    public:
    PngTextWriter(const std::vector<std::string>& paragraphs, const std::string& filename) {
        // PngTextWriter parameters
        this->paragraphs = paragraphs;
        this->filename = filename;

        // Image dimensions
        aspectRatioWidth = ConfigHandler::getInstance().getConfigValue("settings", "aspect_ratio_width");
        aspectRatioHeight = ConfigHandler::getInstance().getConfigValue("settings", "aspect_ratio_height");
        aspectRatio = (float) aspectRatioWidth / aspectRatioHeight;
        height = 1000; // Initial height
        width = round(height * aspectRatio); // Initial width

        // Image margins
        margins = ConfigHandler::getInstance().getConfigValue("settings", "margins");
        leftMargin = round(width * margins);
        rightMargin = round(width * (1.0 - margins));
        bottomMargin = round(height * margins);
        topMargin = round(height * (1.0 - margins));
        horizontalMarginSize = rightMargin - leftMargin;
        verticalMarginSize = topMargin - bottomMargin;

        // Image Font
        std::string fontPath = ConfigHandler::getInstance().getConfigValue("settings", "font_path");
        font = new char[fontPath.length() + 1];
        strcpy(font, fontPath.c_str());
        fontSize = ConfigHandler::getInstance().getConfigValue("settings", "font_size");
        red = ConfigHandler::getInstance().getConfigValue("settings", "font_color", "red");
        green = ConfigHandler::getInstance().getConfigValue("settings", "font_color", "green");
        blue = ConfigHandler::getInstance().getConfigValue("settings", "font_color", "blue");

        // Spacing Settings
        alwaysUseDescenderSpacing = ConfigHandler::getInstance().getConfigValue("settings", "always_use_descender_spacing");
        descenders = "gjpqy";
        descenderSpacing = 30;
        spacing = 10;
        paragraphSpacing = 30;
        spaceWidth = image.get_text_width_utf8(font, fontSize, " ");

        // Tracking Variables
        startHeight = topMargin - fontSize;
        std::string line = "";
        runningWidth = 0;
        lineNumber = 0;
        cursor = topMargin - fontSize;

        // Debugging
        showLineBorders = ConfigHandler::getInstance().getConfigValue("debug_settings", "show_line_borders");
    }

    ~PngTextWriter() {
        delete[] font;
    }

    void writeText() {
        float backgroundColor = 0.0;
        
        image = pngwriter(width, height, backgroundColor, filename.c_str());
        int textHeight = fitImage();
        cursor = topMargin - round((verticalMarginSize - textHeight) / 2) - fontSize;
        
        int paragraphCount = paragraphs.size();
        for (int j = 0; j < paragraphCount; j++) {
            std::vector<std::string> words = StringUtils::splitSentence(paragraphs.at(j));
            line = "";
            runningWidth = 0;
            
            if (j > 0) {
                cursor -= fontSize + spacing + descenderSpacing;
            }
            
            int wordCount = words.size();
            for (int i = 0; i < wordCount; i++) {
                if (line.length() > 0) {
                    runningWidth += spaceWidth;
                }

                char * word = new char[words.at(i).length() + 1];
                strcpy(word, words.at(i).c_str());
                
                int wordWidth = image.get_text_width_utf8(font, fontSize, word);
                
                runningWidth += wordWidth;
                
                if (runningWidth <= horizontalMarginSize) {
                    if (line.length() > 0) {
                        line += " ";
                    }
                    
                    line += words.at(i);
                    if (i + 1 == wordCount) {
                        writeLine(showLineBorders);
                    }
                } else {
                    runningWidth -= (spaceWidth + wordWidth);
                    
                    writeLine(showLineBorders);
                    
                    lineNumber += 1;
                    line = "";
                    i -= 1;
                    runningWidth = 0;
                }
                
                delete[] word;
            }
        }
        
        std::cout << "Successfully generated image." << std::endl;
        
        saveAndClose();
        
        return;
    }

    private:
    std::vector<std::string> paragraphs;
    std::string filename;
    int aspectRatioWidth;
    int aspectRatioHeight;
    float aspectRatio;
    int height;
    int width;
    float margins;
    int leftMargin;
    int rightMargin;
    int bottomMargin;
    int topMargin;
    int horizontalMarginSize;
    int verticalMarginSize;
    char* font;
    int fontSize;
    float red;
    float green;
    float blue;
    const char * descenders;
    bool alwaysUseDescenderSpacing;
    int descenderSpacing;
    int spacing;
    int paragraphSpacing;
    int startHeight;
    int spaceWidth;
    std::string line;
    int runningWidth;
    int lineNumber;
    int cursor;
    bool showLineBorders;
    pngwriter image;

    void setDimensions(int newHeight) {
        height = newHeight;
        width = round(height * aspectRatio);
        leftMargin = round(width * margins);
        rightMargin = round(width * (1.0 - margins));
        bottomMargin = round(height * margins);
        topMargin = round(height * (1.0 - margins));
        horizontalMarginSize = rightMargin - leftMargin;
        verticalMarginSize = topMargin - bottomMargin;
        image.resize(width, height);
    };
    
    int fitImage() {
        int textHeight = fontSize + descenderSpacing;
        int paragraphCount = paragraphs.size();
        int heightDelta = 100;

        for (int j = 0; j < paragraphCount; j++) {
            
            std::vector<std::string> words = StringUtils::splitSentence(paragraphs.at(j));
            line = "";
            runningWidth = 0;

            if (j > 0) {
                textHeight += fontSize + descenderSpacing + spacing;
            }
            
            int wordCount = words.size();
            for (int i = 0; i < wordCount; i++) {
                if (line.length() > 0) {
                    runningWidth += spaceWidth;
                }

                char * word = new char[words.at(i).length() + 1];
                strcpy(word, words.at(i).c_str());
                
                int wordWidth = image.get_text_width_utf8(font, fontSize, word);
                
                if (wordWidth > horizontalMarginSize) {
                    delete[] word;
                    setDimensions(height + heightDelta);
                    textHeight = fitImage();
                    return textHeight;
                }
                
                runningWidth += wordWidth;
                
                if (runningWidth <= horizontalMarginSize) {
                    if (line.length() > 0) {
                        line += " ";
                    }
                    
                    line += words.at(i);
                    if (i + 1 == wordCount && j + 1 < paragraphCount) {
                        textHeight += fontSize + descenderSpacing + spacing;
                    }
                }
                else {
                    runningWidth -= (spaceWidth + wordWidth);
                    
                    textHeight += fontSize + descenderSpacing + spacing;
                    
                    line = "";
                    i -= 1;
                    runningWidth = 0;
                }
                
                delete[] word;
            }
        }
        
        if (textHeight > verticalMarginSize) {
            setDimensions(height + heightDelta);
            textHeight = fitImage();
        }
        
        return textHeight;
        
    };
    
    void writeLine(bool createLines = false) {
        bool lineHasDescenders = StringUtils::stringContainsElement(line, descenders);
        
        char * lineCopy = new char[line.length() + 1];
        strcpy(lineCopy, line.c_str());
        
        int lineHorizontalStart = leftMargin + round((horizontalMarginSize - runningWidth) / 2);
        
        if (createLines) {
            int yFrom = cursor - (lineHasDescenders * descenderSpacing);
            int yTo = cursor + fontSize;
            image.square(lineHorizontalStart, yFrom, lineHorizontalStart + runningWidth, yTo, 1.0, 0.0, 0.0);
        }
        
        image.plot_text_utf8(font, fontSize, lineHorizontalStart, cursor, 0.0, lineCopy, red, green, blue);
        delete[] lineCopy;
        
        cursor -= fontSize + spacing + ((lineHasDescenders || alwaysUseDescenderSpacing) * descenderSpacing);
    };
    
    void saveAndClose() {
        image.close();
        std::cout << "Image successfully saved and closed." << std::endl;
    }
    
};

#endif