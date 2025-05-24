#ifndef PNG_TEXT_WRITER_H
#define PNG_TEXT_WRITER_H

#include <pngwriter.h>
#include <vector>
#include <string>

class PngTextWriter {
    
public:
    PngTextWriter(const std::vector<std::string>& paragraphs, const std::string& filename);
    ~PngTextWriter();
    void writeText();

private:
    void setDimensions(int newHeight);
    void increaseImageSize();
    int getTextWidth(const std::string& word);
    bool fitsInHeight(int textHeight);
    bool fitsInWidth(int textWidth);
    bool lineHasDescenders(const std::string& line);
    int getLineHeight(const std::string& line);
    bool isFirstLineOfText(int lineNumber, int paragraphNumber);
    bool isFirstParagraph(int paragraphNumber);
    bool isLastWordInParagraph(int wordNumber, int wordCount);
    int calculateSpacing(const std::string& line, int lineNumber, int paragraphNumber);
    std::vector<std::vector<std::string>> fitImage();
    bool fitImage(std::vector<std::vector<std::string>>& textSegments);
    void drawText(int startX, int startY, const std::string& line);
    void drawBoxAroundText(int startX, int startY, int lineWidth, bool hasDescenders);
    void writeLine(const std::string& line, int startY);
    void saveAndClose();
    
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

};

#endif //PNG_TEXT_WRITER_H