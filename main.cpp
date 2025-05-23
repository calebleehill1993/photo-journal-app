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

int main() {
    const std::string project_path = FileUtils::getExecutableDirectory();

    FileUtils::setCurrentPath(project_path);

    GoogleAPIHandler googleAPIHandler = GoogleAPIHandler();;

    std::vector<std::unique_ptr<EntryExtractor>> entryExtractors;
    entryExtractors.push_back(std::make_unique<GoogleDocsEntryExtractor>(googleAPIHandler.getDoc()));
    
    std::vector<Entry> entries;
    
    for (auto& extractor : entryExtractors) {
        auto extractedEntries = extractor->extract_entries(); // Use -> to call the method on the pointer
        entries.insert(entries.end(), extractedEntries.begin(), extractedEntries.end());
    }
    
    std::vector<std::vector<std::string>> rows;
    
    for (Entry entry : entries) {

        PngTextWriter pngTextWriter(entry.getTitle(), entry.getDate(), entry.getTime(), entry.to_filename());
        pngTextWriter.write_text();
        
        std::string filename = entry.to_filename();
        
        FileUtils::update_exif_original_date(project_path + filename, entry.get_exif_datetime(), entry.getTimeOffset());

        std::string photosId = googleAPIHandler.uploadPhoto(project_path, filename, entry.generatePhotosDescription());
        entry.setPhotosId(photosId);
        FileUtils::deleteFile(filename);

        rows.push_back(entry.to_vector());
    }

    googleAPIHandler.appendRowsToSheet(rows);
    
    return 0;
}