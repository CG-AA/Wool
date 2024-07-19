#ifndef WOOL_HPP
#define WOOL_HPP

#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>

typedef websocketpp::client<websocketpp::config::asio_tls_client> ws_client;

class Wool {
private:
    std::string PUBKEY;
    std::string token;
    std::string WSS_URL;

    CURL *curl;
    CURLcode res;
    std::string readBuffer;

    ws_client uWS;

    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
public:

    void setPUBKEY(std::string pubkey){
        this->PUBKEY = pubkey;}
    void setToken(std::string token){
        this->token = token;}

    Wool(); // Constructor
    ~Wool(); // Destructor

    void connect_ws();

    void sendMsg(std::string msg, int64_t channelID, bool allowMention = true);
};

#endif // WOOL_HPP