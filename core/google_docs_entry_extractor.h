#ifndef GOOGLE_DOCS_ENTRY_EXTRACTOR_H
#define GOOGLE_DOCS_ENTRY_EXTRACTOR_H

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "entry_extractor.h"
#include "entry.h"

class GoogleDocsEntryExtractor : public EntryExtractor {

public:
    GoogleDocsEntryExtractor(const std::string& docJson);
    std::vector<Entry> extractEntries() override;

private:
    std::string getTextFromElements(const nlohmann::json& elements, bool& isBold);
    void resetForNewEntry();
    std::string adjustedDefaultTimeOffset(const std::string& currentDate, const std::string& currentTime);

private:
    std::string docJson;
    std::string defaultTime;
    std::string defaultTimezoneOffset;
    bool adjustForDaylightSavings;
    std::string currentDate;
    std::string currentTime;
    std::string currentTimeOffset;
    std::vector<std::string> currentTags;
    std::vector<std::string> currentTitle;
    std::vector<std::string> currentBody;
};

#endif // GOOGLE_DOCS_ENTRY_EXTRACTOR_H
