#ifndef GOOGLE_AUTH_H
#define GOOGLE_AUTH_H

#include <string>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <curl/curl.h>
#include "../config/config_handler.h"
#include "../utils/web_utils.h"

using json = nlohmann::json;
namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
using tcp = asio::ip::tcp;
using namespace std;

class GoogleAuth {
public:
    static GoogleAuth& getInstance() {
        static GoogleAuth instance;
        return instance;
    }

    // Delete copy constructor and assignment operator to enforce singleton
    GoogleAuth(const GoogleAuth&) = delete;
    GoogleAuth& operator=(const GoogleAuth&) = delete;

    void openAuthenticationPage() {
        const string authURL = "https://accounts.google.com/o/oauth2/auth?client_id=" + clientId +
                               "&redirect_uri=" + redirectUri +
                               "&response_type=code" +
                               "&scope=https://www.googleapis.com/auth/photoslibrary.appendonly" +
                               " https://www.googleapis.com/auth/drive";
        
        cout << "Sending User to Authentication URL" << endl << authURL << endl;
        
        SystemOpenURL(authURL);
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

    // Function to handle HTTP requests
    http::request<http::string_body> handle_request(tcp::socket &socket) {
        beast::flat_buffer buffer;
        http::request<http::string_body> request;
        http::read(socket, buffer, request);

        // Send response to browser
        http::response<http::string_body> response{http::status::ok, request.version()};
        response.set(http::field::content_type, "text/html");
        response.body() = "<h1>Authorization Successful</h1><p>You can close this window.</p>";
        response.prepare_payload();
        http::write(socket, response);

        return request;
    }

    // Function to start a basic HTTP server
    http::request<http::string_body> start_local_http_server() {
        asio::io_context ioc;
        tcp::acceptor acceptor(ioc, tcp::endpoint(tcp::v4(), 8080));

        std::cout << "Waiting for OAuth response at " + redirectUri + " ..." << std::endl;

        tcp::socket socket(ioc);
        acceptor.accept(socket);
        return handle_request(socket);
    }

    void getAuthorizationCode() {
        openAuthenticationPage();

        http::request<http::string_body> request = start_local_http_server();
        std::string targetUrl = std::string(request.target());
        std::size_t pos = targetUrl.find("code=");

        if (pos != std::string::npos) {
            std::size_t endPos = targetUrl.find("&", pos);
            authorizationCode = targetUrl.substr(pos + 5, endPos - (pos + 5));  // Extract the OAuth code
            std::cout << "Authorization Code Received: " << authorizationCode << std::endl;
        } else {
            std::cerr << "Authorization code not found in the URL." << std::endl;
        }
    }

    void getNewRefreshToken() {
        redirectUri = ConfigHandler::getInstance().getConfigValue("google_auth", "redirect_uri");

        getAuthorizationCode();

        std::string command = "curl -X POST \"https://oauth2.googleapis.com/token\" "
                            "-H \"Content-Type: application/x-www-form-urlencoded\" "
                            "-d \"code=" + authorizationCode +
                            "&client_id=" + clientId +
                            "&client_secret=" + clientSecret +
                            "&redirect_uri=" + redirectUri +
                            "&grant_type=authorization_code\"";

        std::string response;
        char buffer[128];
        FILE *pipe = popen(command.c_str(), "r");
        if (!pipe) {
            std::cerr << "Error running cURL command!" << std::endl;
        }
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            response += buffer;
        }
        pclose(pipe);

        saveRefreshToken(response);
    }

    void saveRefreshToken(const std::string& response) {
        json jsonResponse = json::parse(response);
        refreshToken = jsonResponse["refresh_token"];
        std::cout << "Refresh Token: " << refreshToken << std::endl;

        // Save the refresh token to the config file
        ConfigHandler::getInstance().setConfigValue("google_auth", "refresh_token", refreshToken.c_str());
    }

    // Exchange the authorization code for an access token
    void getRefreshToken() {
        try {
            refreshToken = ConfigHandler::getInstance().getConfigValue("google_auth", "refresh_token");
        } catch (const std::runtime_error& e) {
            std::cout << "Refresh token not found. Initiating authorization process to retrieve it." << std::endl;
            getNewRefreshToken();
        }
    }

    // Function to refresh the access token
    std::string getNewAccessToken() {
        if (refreshToken.empty()) {
            getRefreshToken();
        }

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
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WebUtils::WriteCallback);
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

    std::string getAccessToken() {
        if (!accessToken.empty()) {
            return accessToken;
        } else {
            accessToken = getNewAccessToken();
            return accessToken;
        }
    }

private:
    GoogleAuth() {
        clientId = ConfigHandler::getInstance().getConfigValue("google_auth", "client_id");
        clientSecret = ConfigHandler::getInstance().getConfigValue("google_auth", "client_secret");
    }
    ~GoogleAuth() = default; // Private destructor

    // Private members for authentication
    std::string clientId;
    std::string clientSecret;
    std::string authorizationCode;
    std::string refreshToken;
    std::string accessToken;
    std::string redirectUri;
};

#endif // GOOGLE_AUTH_H
