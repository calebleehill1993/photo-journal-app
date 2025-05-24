#ifndef PNG_TEXT_WRITER_H
#define PNG_TEXT_WRITER_H

#include <pngwriter.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <string>
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
        textAreaWidth = rightMargin - leftMargin;
        textAreaHeight = topMargin - bottomMargin;

        // Image Font Settings
        std::string fontPath = ConfigHandler::getInstance().getConfigValue("settings", "font_path");
        font = new char[fontPath.length() + 1];
        strcpy(font, fontPath.c_str());
        fontSize = ConfigHandler::getInstance().getConfigValue("settings", "font_size");
        red = ConfigHandler::getInstance().getConfigValue("settings", "font_color", "red");
        green = ConfigHandler::getInstance().getConfigValue("settings", "font_color", "green");
        blue = ConfigHandler::getInstance().getConfigValue("settings", "font_color", "blue");
        textAlignment = ConfigHandler::getInstance().getConfigValue("settings", "text_alignment");

        // Spacing Settings
        alwaysUseDescenderSpacing = ConfigHandler::getInstance().getConfigValue("settings", "always_use_descender_spacing");
        descenders = "gjpqy";
        descenderSpacing = 30;
        lineSpacing = 10;
        paragraphSpacing = fontSize + descenderSpacing + lineSpacing;
        spaceWidth = image.get_text_width_utf8(font, fontSize, " ");

        // Tracking Variables
        textHeight = 0;
        paragraphCount = paragraphs.size();
        heightDelta = 100;

        // Debugging
        showLineBorders = ConfigHandler::getInstance().getConfigValue("debug_settings", "show_line_borders");
    }

    ~PngTextWriter() {
        delete[] font;
    }

    void writeText() {
        float backgroundColor = 0.0;
        
        image = pngwriter(width, height, backgroundColor, filename.c_str());
        std::vector<std::vector<std::string>> textSegments = fitImage();
        int cursor = topMargin - round((textAreaHeight - textHeight) / 2) - fontSize;

        for (auto& paragraph : textSegments) {
            for (auto& line : paragraph) {
                writeLine(line, cursor);
                cursor -= lineSpacing + getLineHeight(line);
            }
            cursor -= paragraphSpacing;
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
    int textAreaWidth;
    int textAreaHeight;
    char* font;
    int fontSize;
    float red;
    float green;
    float blue;
    std::string textAlignment;
    const char * descenders;
    bool alwaysUseDescenderSpacing;
    int descenderSpacing;
    int lineSpacing;
    int paragraphSpacing;
    int spaceWidth;
    int textHeight;
    int paragraphCount;
    int heightDelta;
    bool showLineBorders;
    pngwriter image;

    void setDimensions(int newHeight) {
        height = newHeight;
        width = round(height * aspectRatio);
        leftMargin = round(width * margins);
        rightMargin = round(width * (1.0 - margins));
        bottomMargin = round(height * margins);
        topMargin = round(height * (1.0 - margins));
        textAreaWidth = rightMargin - leftMargin;
        textAreaHeight = topMargin - bottomMargin;
        image.resize(width, height);
    };
    
    void increaseImageSize() {
        setDimensions(height + heightDelta);
    };

    int getTextWidth(const std::string& word) {
        char * wordCopy = new char[word.length() + 1];
        strcpy(wordCopy, word.c_str());
        
        int wordWidth = image.get_text_width_utf8(font, fontSize, wordCopy);
        
        delete[] wordCopy;
        
        return wordWidth;
    };

    bool fitsInHeight(int textHeight) {
        if (textHeight > textAreaHeight) {
            return false;
        }
        
        return true;
    };

    bool fitsInWidth(int textWidth) {
        if (textWidth > textAreaWidth) {
            return false;
        }
        
        return true;
    }

    bool lineHasDescenders(std::string& line) {
        return StringUtils::stringContainsElement(line, descenders);
    }

    int getLineHeight(std::string& line) {
        return fontSize + (lineHasDescenders(line) || alwaysUseDescenderSpacing) * descenderSpacing;
    }

    bool isFirstLineOfText(int lineNumber, int paragraphNumber) {
        return (lineNumber == 0 && paragraphNumber == 0);
    }

    bool isFirstParagraph(int paragraphNumber) {
        return (paragraphNumber == 0);
    }

    bool isLastWordInParagraph(int wordNumber, int wordCount) {
        return (wordNumber + 1 == wordCount);
    }

    int calculateSpacing(std::string& line, int lineNumber, int paragraphNumber) {
        if (isFirstLineOfText(lineNumber, paragraphNumber)) {
            return getLineHeight(line);
        }
        
        return lineSpacing + getLineHeight(line);
    }

    std::vector<std::vector<std::string>> fitImage() {
        textHeight = 0;
        std::vector<std::vector<std::string>> textSegments;

        for (int j = 0; j < paragraphCount; j++) {
            std::vector<std::string> words = StringUtils::splitSentence(paragraphs.at(j));
            std::string line = "";
            int runningLineWidth = 0;
            int lineCount = 0;
            textSegments.push_back(std::vector<std::string>());

            if (!isFirstParagraph(j)) {
                textHeight += paragraphSpacing;

                if (!fitsInHeight(textHeight)) {
                    increaseImageSize();
                    return fitImage();
                }
            }
            
            int paragraphWordCount = words.size();
            for (int i = 0; i < paragraphWordCount; i++) {
                // Adding a space between words
                if (line.length() > 0) {
                    runningLineWidth += spaceWidth;
                }

                int wordWidth = getTextWidth(words.at(i));
                
                // Single word is too big for the text area
                if (!fitsInWidth(wordWidth)) {
                    increaseImageSize();
                    return fitImage();
                }
                
                runningLineWidth += wordWidth;
                
                if (fitsInWidth(runningLineWidth)) {
                    if (line.length() > 0) {
                        line += " ";
                    }
                    
                    line += words.at(i);

                    if (isLastWordInParagraph(i, paragraphWordCount)) {
                        
                        textHeight += calculateSpacing(line, lineCount, j);
                        textSegments.at(j).push_back(line);

                        if (!fitsInHeight(textHeight)) {
                            increaseImageSize();
                            return fitImage();
                        }
                    }
                } else {
                    runningLineWidth -= (spaceWidth + wordWidth);

                    textHeight += calculateSpacing(line, lineCount, j);
                    textSegments.at(j).push_back(line);

                    if (!fitsInHeight(textHeight)) {
                        increaseImageSize();
                        return fitImage();
                    }
                    
                    lineCount += 1;
                    line = "";
                    i -= 1; // Try the same word on the next line
                    runningLineWidth = 0;
                }
            }
        }
        
        return textSegments;
        
    };

    void drawText(int startX, int startY, const std::string& line) {
        char * lineCopy = new char[line.length() + 1];
        strcpy(lineCopy, line.c_str());
        image.plot_text_utf8(font, fontSize, startX, startY, 0.0, lineCopy, red, green, blue);
        delete[] lineCopy;
    }

    void drawBoxAroundText(int startX, int startY, int lineWidth, bool hasDescenders) {
        startY = startY - (hasDescenders * descenderSpacing);
        int endY = startY + fontSize;
        int endX = startX + lineWidth;
        image.square(startX, startY, endX, endY, 1.0, 0.0, 0.0);
    }
    
    void writeLine(std::string& line, int startY) {
        int startX = 0;

        int lineWidth = getTextWidth(line);
        
        if (textAlignment == "center") {
            startX = leftMargin + round((textAreaWidth - lineWidth) / 2);
        } else if (textAlignment == "right") {
            startX = rightMargin - lineWidth;
        } else if (textAlignment == "left") {
            startX = leftMargin;
        } else {
            std::cerr << "Invalid alignment option. Defaulting to left." << std::endl;
            startX = leftMargin;
        }
        
        if (showLineBorders) {
            drawBoxAroundText(startX, startY, lineWidth, lineHasDescenders(line));
        }
        
        drawText(startX, startY, line);
    };
    
    void saveAndClose() {
        image.close();
        std::cout << "Image successfully saved and closed." << std::endl;
    }
    
};

#endif