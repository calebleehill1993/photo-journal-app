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
     -lpngwriter -lpng -lfreetype -lcurl -lexiv2 \
     -I/opt/homebrew/include -I/opt/homebrew/include/freetype2 \
     -L/opt/homebrew/lib
```

#### 2. Run the created execution file.