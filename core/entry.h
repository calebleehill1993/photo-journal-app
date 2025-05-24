#ifndef ENTRY_H
#define ENTRY_H

#include <string>
#include <vector>

class Entry {
    
public:
    Entry(std::string date, std::string time, std::string timeOffset, std::vector<std::string> tags, std::vector<std::string> title, std::vector<std::string> body, std::string entryType);
    void setPhotosId(std::string& photosId);
    std::string getPhotosId();
    std::string getDate();
    std::string getTime();
    std::string getTimeOffset();
    std::vector<std::string> getTags();
    std::vector<std::string> getTitle();
    std::vector<std::string> getBody();
    std::string getEntryType();
    std::string toFilename();
    std::string toTimestamp();
    std::string getExifDatetime();
    std::string getFirstNCharsFromParagraphs(const std::vector<std::string>& paragraphs, int charsNeeded);
    std::string generateId();
    std::string generatePhotosDescription();
    std::string toString();
    std::vector<std::string> toVector();
    std::string getTitleString(const std::string& delimiter = "\n\n");
    std::string getBodyString(const std::string& delimiter = "\n\n");
    std::string getTagsString(const std::string& delimiter = "|");

private:
    std::string formattedId(const std::string& id);
    std::string vectorToString(const std::vector<std::string>& vec, const std::string& delimiter = "|");

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
    
};

#endif
