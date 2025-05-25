#include "png_text_writer.h"
#include <cmath>
#include <iostream>
#include "../utils/string_utils.h"
#include "../config/config_handler.h"

namespace ConfigConst = ConfigConstants;

const std::string LEFT = "left";
const std::string CENTER = "center";
const std::string RIGHT = "right";
const std::string DESCENDERS = "gjpqy";

PngTextWriter::PngTextWriter(const std::vector<std::string>& paragraphs, const std::string& filename) {
    // PngTextWriter parameters
    this->paragraphs = paragraphs;
    this->filename = filename;

    // Image dimensions
    aspectRatioWidth = ConfigHandler::getInstance().getConfigValue(ConfigConst::SETTINGS, ConfigConst::ASPECT_RATIO_WIDTH);
    aspectRatioHeight = ConfigHandler::getInstance().getConfigValue(ConfigConst::SETTINGS, ConfigConst::ASPECT_RATIO_HEIGHT);
    aspectRatio = (float) aspectRatioWidth / aspectRatioHeight;
    height = 1000; // Initial height
    width = round(height * aspectRatio); // Initial width

    // Image margins
    margins = ConfigHandler::getInstance().getConfigValue(ConfigConst::SETTINGS, ConfigConst::MARGINS);
    leftMargin = round(width * margins);
    rightMargin = round(width * (1.0 - margins));
    bottomMargin = round(height * margins);
    topMargin = round(height * (1.0 - margins));
    textAreaWidth = rightMargin - leftMargin;
    textAreaHeight = topMargin - bottomMargin;

    // Image Font Settings
    std::string fontPath = ConfigHandler::getInstance().getConfigValue(ConfigConst::SETTINGS, ConfigConst::FONT_PATH);
    font = new char[fontPath.length() + 1];
    strcpy(font, fontPath.c_str());
    fontSize = ConfigHandler::getInstance().getConfigValue(ConfigConst::SETTINGS, ConfigConst::FONT_SIZE);
    red = ConfigHandler::getInstance().getConfigValue(ConfigConst::SETTINGS, ConfigConst::FONT_COLOR, ConfigConst::RED);
    green = ConfigHandler::getInstance().getConfigValue(ConfigConst::SETTINGS, ConfigConst::FONT_COLOR, ConfigConst::GREEN);
    blue = ConfigHandler::getInstance().getConfigValue(ConfigConst::SETTINGS, ConfigConst::FONT_COLOR, ConfigConst::BLUE);
    textAlignment = ConfigHandler::getInstance().getConfigValue(ConfigConst::SETTINGS, ConfigConst::TEXT_ALIGNMENT);

    // Spacing Settings
    alwaysUseDescenderSpacing = ConfigHandler::getInstance().getConfigValue(ConfigConst::SETTINGS, ConfigConst::ALWAYS_USE_DESCENDER_SPACING);
    descenderSpacing = 30;
    lineSpacing = 10;
    paragraphSpacing = fontSize + descenderSpacing + lineSpacing;
    spaceWidth = image.get_text_width_utf8(font, fontSize, " ");

    // Tracking Variables
    textHeight = 0;
    paragraphCount = paragraphs.size();
    heightDelta = 100;

    // Debugging
    showLineBorders = ConfigHandler::getInstance().getConfigValue(ConfigConst::DEBUG_SETTINGS, ConfigConst::SHOW_LINE_BORDERS);
}

PngTextWriter::~PngTextWriter() {
    delete[] font;
}

void PngTextWriter::writeText() {
    float backgroundColor = 0.0;
    
    image = pngwriter(width, height, backgroundColor, filename.c_str());
    std::vector<std::vector<std::string>> textSegments = fitImage();
    int cursor = topMargin - round((textAreaHeight - textHeight) / 2) - fontSize;

    for (const auto& paragraph : textSegments) {
        for (const auto& line : paragraph) {
            writeLine(line, cursor);
            cursor -= lineSpacing + getLineHeight(line);
        }
        cursor -= paragraphSpacing;
    }
    
    saveAndClose();
}

void PngTextWriter::setDimensions(int newHeight) {
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

void PngTextWriter::increaseImageSize() {
    setDimensions(height + heightDelta);
};

int PngTextWriter::getTextWidth(const std::string& word) {
    char * wordCopy = new char[word.length() + 1];
    strcpy(wordCopy, word.c_str());
    
    int wordWidth = image.get_text_width_utf8(font, fontSize, wordCopy);
    
    delete[] wordCopy;
    
    return wordWidth;
};

bool PngTextWriter::fitsInHeight(int textHeight) {
    if (textHeight > textAreaHeight) {
        return false;
    }
    
    return true;
};

bool PngTextWriter::fitsInWidth(int textWidth) {
    if (textWidth > textAreaWidth) {
        return false;
    }
    
    return true;
}

bool PngTextWriter::lineHasDescenders(const std::string& line) {
    return StringUtils::stringContainsElement(line, DESCENDERS);
}

int PngTextWriter::getLineHeight(const std::string& line) {
    return fontSize + ((lineHasDescenders(line) || alwaysUseDescenderSpacing) ? descenderSpacing : 0);
}

bool PngTextWriter::isFirstLineOfText(int lineNumber, int paragraphNumber) {
    return (lineNumber == 0 && paragraphNumber == 0);
}

bool PngTextWriter::isFirstParagraph(int paragraphNumber) {
    return (paragraphNumber == 0);
}

bool PngTextWriter::isLastWordInParagraph(int wordNumber, int wordCount) {
    return (wordNumber + 1 == wordCount);
}

int PngTextWriter::calculateSpacing(const std::string& line, int lineNumber, int paragraphNumber) {
    if (isFirstLineOfText(lineNumber, paragraphNumber)) {
        return getLineHeight(line);
    }
    
    return lineSpacing + getLineHeight(line);
}

std::vector<std::vector<std::string>> PngTextWriter::fitImage() {
    std::vector<std::vector<std::string>> textSegments;
    bool imageBigEnough = false;
    while (!imageBigEnough) {
        imageBigEnough = fitImage(textSegments);
        if (!imageBigEnough) {
            textSegments.clear();
            increaseImageSize();
        }
    }
    return textSegments;
}

bool PngTextWriter::fitImage(std::vector<std::vector<std::string>>& textSegments) {
    textHeight = 0;

    for (int j = 0; j < paragraphCount; j++) {
        std::vector<std::string> words = StringUtils::splitSentence(paragraphs.at(j));
        std::string line = "";
        int runningLineWidth = 0;
        int lineCount = 0;
        textSegments.push_back(std::vector<std::string>());

        if (!isFirstParagraph(j)) {
            textHeight += paragraphSpacing;

            if (!fitsInHeight(textHeight)) {
                return false;
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
                return false;
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
                        return false;
                    }
                }
            } else {
                runningLineWidth -= (spaceWidth + wordWidth);

                textHeight += calculateSpacing(line, lineCount, j);
                textSegments.at(j).push_back(line);

                if (!fitsInHeight(textHeight)) {
                    return false;
                }
                
                lineCount += 1;
                line = "";
                i -= 1; // Try the same word on the next line
                runningLineWidth = 0;
            }
        }
    }

    return true;
    
};

void PngTextWriter::drawText(int startX, int startY, const std::string& line) {
    char * lineCopy = new char[line.length() + 1];
    strcpy(lineCopy, line.c_str());
    image.plot_text_utf8(font, fontSize, startX, startY, 0.0, lineCopy, red, green, blue);
    delete[] lineCopy;
}

void PngTextWriter::drawBoxAroundText(int startX, int startY, int lineWidth, bool hasDescenders) {
    startY = startY - (hasDescenders * descenderSpacing);
    int endY = startY + fontSize;
    int endX = startX + lineWidth;
    image.square(startX, startY, endX, endY, 1.0, 0.0, 0.0);
}

void PngTextWriter::writeLine(const std::string& line, int startY) {
    int startX = 0;

    int lineWidth = getTextWidth(line);
    
    if (textAlignment == CENTER) {
        startX = leftMargin + round((textAreaWidth - lineWidth) / 2);
    } else if (textAlignment == RIGHT) {
        startX = rightMargin - lineWidth;
    } else if (textAlignment == LEFT) {
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

void PngTextWriter::saveAndClose() {
    image.close();
}