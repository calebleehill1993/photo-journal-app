/*
A Simple Journal Entry Orgnaizer using Google Photos and other Google APIs

By: Caleb Hill

This program extracts journal entries from Google Docs,
generates PNG images for each entry, uploads them to Google Photos,
and loads the entries to a Google Sheet.
*/

#include <iostream>
#include <string>
#include <vector>
#include "config/config_handler.h"
#include "core/entry.h"
#include "core/entry_extractor.h"
#include "core/google_docs_entry_extractor.h"
#include "core/png_text_writer.h"
#include "api/google_api_handler.h"
#include "utils/file_utils.h"

std::vector<Entry> extractEntries(GoogleAPIHandler& googleAPIHandler) {
    std::vector<std::unique_ptr<EntryExtractor>> entryExtractors;
    entryExtractors.push_back(std::make_unique<GoogleDocsEntryExtractor>(googleAPIHandler.getDoc()));
    
    std::vector<Entry> entries;
    
    for (auto& extractor : entryExtractors) {
        auto extractedEntries = extractor->extractEntries();
        entries.insert(entries.end(), extractedEntries.begin(), extractedEntries.end());
    }

    return entries;
}

void processEntries(const std::string& projectPath, std::vector<Entry>& entries, GoogleAPIHandler& googleAPIHandler) {
    
    std::vector<std::vector<std::string>> rowEntries;
    int entryCount = entries.size();
    int currentEntry = 0;
    for (Entry& entry : entries) {
        std::cout << std::endl << "Processing entry " << ++currentEntry << " of " << entryCount << std::endl;

        std::string filename = entry.toFilename();
        PngTextWriter pngTextWriter(entry.getTitle(), filename);
        pngTextWriter.writeText();
        
        FileUtils::updateExifOriginalDate(projectPath + filename, entry.getExifDatetime(), entry.getTimeOffset());

        std::string photosId = googleAPIHandler.uploadPhoto(projectPath, filename, entry.generatePhotosDescription());
        entry.setPhotosId(photosId);
        FileUtils::deleteFile(filename);

        rowEntries.push_back(entry.toVector());
    }

    googleAPIHandler.appendRowsToSheet(rowEntries);
}

int main() {
    const std::string projectPath = FileUtils::getExecutableDirectory();
    FileUtils::setCurrentPath(projectPath);

    GoogleAPIHandler googleAPIHandler = GoogleAPIHandler();
    std::vector<Entry> entries = extractEntries(googleAPIHandler);
    processEntries(projectPath, entries, googleAPIHandler);
    
    return 0;
}