# photo-journal-app
A Simple Journal Entry Orgnaizer using Google Photos and other Google APIs

By: Caleb Hill

This program extracts journal entries from Google Docs, generates PNG images for each entry, uploads them to Google Photos, and loads the entries to a Google Sheet.

### Installing Dependencies
```
brew install pngwriter
brew install libpng
brew install freetype
brew install curl
brew install exiv2
brew install boost
brew install nlohmann-json
```

### How to run
Building the app is easy once the necessary libraries have been installed. 

#### 1. Compile the application by running the following command in the terminal:
```
g++ -std=c++17 -o main main.cpp \
     core/png_text_writer.cpp \
     core/google_docs_entry_extractor.cpp \
     core/entry.cpp \
     config/config_handler.cpp \
     utils/time_utils.cpp \
     utils/web_utils.cpp \
     utils/string_utils.cpp \
     utils/config_utils.cpp \
     utils/file_utils.cpp \
     api/google_sheets.cpp \
     api/google_docs.cpp \
     api/google_api_handler.cpp \
     api/google_drive.cpp \
     api/google_auth.cpp \
     api/google_photos.cpp \
     -lpngwriter -lpng -lfreetype -lcurl -lexiv2 \
     -I/opt/homebrew/include -I/opt/homebrew/include/freetype2 \
     -L/opt/homebrew/lib
```

#### 2. Run the created execution file.