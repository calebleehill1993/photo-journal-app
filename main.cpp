/*
 g++ -std=c++17 -o main main.cpp \
      -lpngwriter -lpng -lfreetype -lcurl -lexiv2 \
      -I/opt/homebrew/include -I/opt/homebrew/include/freetype2 \
      -L/opt/homebrew/lib
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <curl/curl.h>
#include <sys/stat.h>
#include <nlohmann/json.hpp>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <cstdlib>
#include <string_view>
#include <cstdio>
#include "pngwriter.h"
#include "docs_json_formatter.h"
#include "image_generator.cpp"
#include <ctime>
#include <iomanip>
#include <exiv2/exiv2.hpp>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/stat.h>
    #include <utime.h>
#endif

using json = nlohmann::json;
namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
using tcp = asio::ip::tcp;
using namespace std;

const string IMAGE_PATH = "/Users/calebhill/Documents/google_photos_api_test/google_photos_api_test/test_photo.png";
const string JSON_CONFIG_PATH = "/Users/calebhill/Documents/google_photos_api_test/google_photos_api_test/config.json";

std::string authorization_code;

json jsonConfig;

void loadConfig() {
    cout << "Loading config file...." << endl;
    
    std::ifstream file(JSON_CONFIG_PATH);
    file >> jsonConfig;
}

void writeJson(const string& fieldName, const string& value) {
    jsonConfig[fieldName] = value;
    
    // Write JSON to file
    std::ofstream outputFile(JSON_CONFIG_PATH);
    if (outputFile.is_open()) {
        outputFile << std::setw(4) << jsonConfig << std::endl;
        outputFile.close();
        std::cout << "JSON data written to config.json" << std::endl;
    } else {
        std::cerr << "Error opening file for writing" << std::endl;
    }
}

void SystemOpenURL(const std::string& url) {
    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        std::string command = "start \"" + url + "\"";
    #elif __APPLE__
        std::string command = "open \"" + url + "\"";
    #elif __linux__
        std::string command = "xdg-open \"" + url + "\"";
    #else
    #error "Unknown compiler"
    #endif
        std::system(command.c_str());
}

void openAuthenticationPage() {
    const string authURL = "https://accounts.google.com/o/oauth2/auth?client_id=" + CLIENT_ID +
                           "&redirect_uri=" + "http://127.0.0.1:8080/" +
                           "&response_type=code" +
                           "&scope=https://www.googleapis.com/auth/photoslibrary.appendonly" +
                           " https://www.googleapis.com/auth/drive";
    
    cout << "Sending User to Authentication URL" << endl << authURL << endl;
    
    SystemOpenURL(authURL);
}

// Function to handle HTTP requests
void handle_request(tcp::socket &socket) {
    beast::flat_buffer buffer;
    http::request<http::string_body> request;
    http::read(socket, buffer, request);

    std::string target = std::string(request.target());
    std::size_t pos = target.find("code=");
    if (pos != std::string::npos) {
        authorization_code = target.substr(pos + 5);  // Extract the OAuth code
        std::cout << "Authorization Code Received: " << authorization_code << std::endl;
    }

    // Send response to browser
    http::response<http::string_body> response{http::status::ok, request.version()};
    response.set(http::field::content_type, "text/html");
    response.body() = "<h1>Authorization Successful</h1><p>You can close this window.</p>";
    response.prepare_payload();
    http::write(socket, response);
}

// Function to start a basic HTTP server
void start_http_server() {
    asio::io_context ioc;
    tcp::acceptor acceptor(ioc, tcp::endpoint(tcp::v4(), 8080));

    std::cout << "Waiting for OAuth response at http://127.0.0.1:8080/ ..." << std::endl;
    
    openAuthenticationPage();

    tcp::socket socket(ioc);
    acceptor.accept(socket);
    handle_request(socket);
}

// Exchange the authorization code for an access token
std::string getRefreshToken(const std::string &code, const std::string &client_id, const std::string &client_secret, const std::string &redirect_uri) {
    std::string command = "curl -X POST \"https://oauth2.googleapis.com/token\" "
                          "-H \"Content-Type: application/x-www-form-urlencoded\" "
                          "-d \"code=" + code +
                          "&client_id=" + client_id +
                          "&client_secret=" + client_secret +
                          "&redirect_uri=" + redirect_uri +
                          "&grant_type=authorization_code\"";

    std::string response;
    char buffer[128];
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe) {
        std::cerr << "Error running cURL command!" << std::endl;
        return "";
    }
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        response += buffer;
    }
    pclose(pipe);

    return response;
}

void getAuthorization() {
    start_http_server();
    
    if (!authorization_code.empty()) {
        std::string client_id = CLIENT_ID;
        std::string client_secret = CLIENT_SECRET;
        std::string redirect_uri = "http://127.0.0.1:8080/";
        
        std::string response = getRefreshToken(authorization_code, client_id, client_secret, redirect_uri);
        std::string refresh_token = json::parse(response)["refresh_token"];
        std::cout << "Refresh Token: " << refresh_token << std::endl;
        
        writeJson("refresh_token", refresh_token);
    }
    
}

bool fileExists(const string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

// Helper function to read file content
string readFile(const string& imagePath, string& filename) {
    
    string fullPath = imagePath + filename;
    
    if (!fileExists(fullPath)) {
        cerr << "File not found: " << fullPath << endl;
        exit(1);
    }
    
    cout << fullPath << endl;
    
    ifstream file(fullPath, ios::binary);
    
    if (!file) {
        cerr << "Error opening file: " << fullPath << endl;
        exit(1);
    }
    
    return string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
}

// Function to handle cURL response
size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* output) {
    size_t total_size = size * nmemb;
    output->append((char*)contents, total_size);
    return total_size;
}

// Function to refresh the access token
string getAccessToken(const string& clientId, const string& clientSecret, const string& refreshToken) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        cerr << "Failed to initialize cURL." << endl;
        return "";
    }

    string response;
    string postFields = "client_id=" + clientId +
                        "&client_secret=" + clientSecret +
                        "&refresh_token=" + refreshToken +
                        "&grant_type=refresh_token";

    curl_easy_setopt(curl, CURLOPT_URL, "https://oauth2.googleapis.com/token");
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        cerr << "cURL request failed: " << curl_easy_strerror(res) << endl;
        return "";
    }

    json jsonResponse = json::parse(response);
    return jsonResponse["access_token"];
}

// Function to upload an image and return the upload token
string uploadImage(const string& accessToken, const string& imagePath, string& filename) {
    CURL* curl;
    CURLcode res;
    string response;
    
    curl = curl_easy_init();
    if (!curl) {
        cerr << "CURL initialization failed!" << endl;
        return "";
    }

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/octet-stream");
    headers = curl_slist_append(headers, "X-Goog-Upload-Protocol: raw");

    string fileData = readFile(imagePath, filename);

    curl_easy_setopt(curl, CURLOPT_URL, "https://photoslibrary.googleapis.com/v1/uploads");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, fileData.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, fileData.size());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        cerr << "Image upload failed: " << curl_easy_strerror(res) << endl;
        return "";
    }

    return response; // Upload token
}

// Function to create a media item
bool createMediaItem(const string& accessToken, const string& uploadToken, const string& filename, const string& description) {
    CURL* curl;
    CURLcode res;
    string response;

//    json requestBody = {
//        {"newMediaItems", {
//            {"description", "This is Jake's Face!!!!"},
//            {"simpleMediaItem", {{"uploadToken", uploadToken},
//                                  {"fileName", "Jake's Face.png"}}}
//        }}
//    };
    

    json requestBody = {
        {"newMediaItems", {
            {
                {"description", description},
                {"simpleMediaItem", {
                    {"uploadToken", uploadToken},
                    {"fileName", filename}
                }}
            }
        }}
    };
    
    cout << endl << "Request: " << requestBody.dump(4) << endl;

    curl = curl_easy_init();
    if (!curl) {
        cerr << "CURL initialization failed!" << endl;
        return false;
    }

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    string postData = requestBody.dump();

    curl_easy_setopt(curl, CURLOPT_URL, "https://photoslibrary.googleapis.com/v1/mediaItems:batchCreate");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        cerr << "Failed to create media item: " << curl_easy_strerror(res) << endl;
        return false;
    }

    cout << "Response: " << response << endl;
    return true;
}

// Function to upload an image and return the upload token
string getDriveFile(const string& fileId, const string& accessToken) {
    CURL* curl;
    CURLcode res;
    string response;
    
    curl = curl_easy_init();
    if (!curl) {
        cerr << "CURL initialization failed!" << endl;
        return "";
    }

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());

    curl_easy_setopt(curl, CURLOPT_URL, ("https://www.googleapis.com/drive/v3/files/" + fileId).c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        cerr << "Get File Failed: " << curl_easy_strerror(res) << endl;
        return "";
    }

    return response; // File Response
}

string getDocFile(const string& docId, const string& accessToken) {
    CURL* curl;
    CURLcode res;
    string response;
    
    curl = curl_easy_init();
    if (!curl) {
        cerr << "CURL initialization failed!" << endl;
        return "";
    }

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());

    curl_easy_setopt(curl, CURLOPT_URL, ("https://docs.googleapis.com/v1/documents/" + docId).c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        cerr << "Get File Failed: " << curl_easy_strerror(res) << endl;
        return "";
    }

    return response; // File Response
}


/**
 * Updates the EXIF "DateTimeOriginal" field in a JPEG image.
 *
 * @param filename The file whose EXIF metadata should be updated.
 * @param timestamp The new timestamp in "YYYY:MM:DD HH:MM:SS" format.
 * @param offset The new offset for the timestamp in "+/-HH:MM" format.
 * @return True if successful, False otherwise.
 */
bool update_exif_original_date(const string& filename, const string& timestamp, const string& offset) {
    try {
        // Open the image file
        Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(filename);
        if (!image) {
            cerr << "Error: Unable to open image file." << endl;
            return false;
        }
        image->readMetadata();

        // Access EXIF metadata
        Exiv2::ExifData& exifData = image->exifData();

        // Modify the "DateTimeOriginal" field
        exifData["Exif.Photo.DateTimeOriginal"] = timestamp;
        exifData["Exif.Photo.OffsetTimeOriginal"] = offset;

        // Save changes
        image->writeMetadata();
        cout << "Successfully updated EXIF DateTimeOriginal to: " << timestamp << endl;
        cout << "Successfully updated EXIF OffsetTimeOriginal to: " << offset << endl;

        return true;
    } catch (const Exiv2::Error& e) {  // Corrected exception handling
        cerr << "EXIF Update Error: " << e.what() << endl;
        return false;
    }
}


int main() {
    loadConfig();
    
    if (!jsonConfig.contains("refresh_token")) {
        getAuthorization();
    }

    string refresh_token = jsonConfig["refresh_token"];
    string CLIENT_ID = jsonConfig["client_id"];
    string CLIENT_SECRET = jsonConfig["client_secret"];
    
    string accessToken = getAccessToken(CLIENT_ID, CLIENT_SECRET, refresh_token);
    
    string fileResponse = getDocFile("1uDcyeROfk7oubtzgjCIFFE4E9dbSbkUIx1m2LxqHiNM", accessToken);
    
    vector<Entry> entries = extract_entries(fileResponse, jsonConfig["default_timezone_offset"], jsonConfig["adjust_for_daylight_savings"]);
    
    for (Entry entry : entries) {
        cout << entry.to_string() << endl;
    }
    
    const string project_path = "/Users/calebhill/Documents/google_photos_api_test/google_photos_api_test/";
    
    filesystem::current_path(project_path);
    
    char* font = "/System/Library/Fonts/Supplemental/Arial Unicode.ttf";
    
    for (Entry entry : entries) {
        
        for (string paragraph : entry.getTitle()){
            cout << paragraph << endl;
        }
        
        PngTextWriter pngtextwriter(entry.getTitle(), entry.getDate(), entry.getTime(), entry.to_filename(), font);
        
        pngtextwriter.write_text();
        
        string filename = entry.to_filename();
        
        string description = "";
        
        if (entry.getEntryType().length() > 0) {
            description = description + "Type: " + entry.getEntryType() + "\n\n";
        }
        
        for (string paragraph : entry.getBody()){
            description = description + paragraph;
        }
        
        string timestamp = entry.getDate() + " " + entry.getTime();  // EXIF format (YYYY:MM:DD HH:MM:SS)
        
        if (update_exif_original_date(project_path + filename, timestamp, entry.getTimeOffset())) {
            cout << "EXIF metadata updated successfully." << endl;
        } else {
            cout << "Failed to update EXIF metadata." << endl;
        }
        
        string uploadToken = uploadImage(accessToken, project_path, filename);
        if (!uploadToken.empty()) {
            cout << "Upload Token: " << uploadToken << endl;
            if (createMediaItem(accessToken, uploadToken, filename, description)) {
                cout << "Image uploaded successfully!" << endl;
            }
        } else {
            cerr << "Failed to upload image." << endl;
        }
        
    }
    
    return 0;
}
