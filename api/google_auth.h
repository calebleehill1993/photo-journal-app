#ifndef GOOGLE_AUTH_H
#define GOOGLE_AUTH_H

#include <string>
#include <boost/asio.hpp>
#include <boost/beast.hpp>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
using tcp = asio::ip::tcp;

class GoogleAuth {
public:
    static GoogleAuth& getInstance();
    std::string getAccessToken();

private:
    GoogleAuth();
    GoogleAuth(const GoogleAuth&) = delete;
    GoogleAuth& operator=(const GoogleAuth&) = delete;

    void openAuthenticationPage();
    void SystemOpenURL(const std::string& url);
    http::request<http::string_body> handle_request(tcp::socket &socket);
    http::request<http::string_body> start_local_http_server();
    void getAuthorizationCode();
    void getNewRefreshToken();
    void saveRefreshToken(const std::string& response);
    void getRefreshToken();
    std::string getNewAccessToken();

private:
    std::string clientId;
    std::string clientSecret;
    std::string authorizationCode;
    std::string refreshToken;
    std::string accessToken;
    std::string redirectUri;

};

#endif // GOOGLE_AUTH_H
