#ifndef WOOL_HPP
#define WOOL_HPP

#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>

typedef websocketpp::client<websocketpp::config::asio_tls_client> ws_client;

class Wool {
friend class WoolHelper;
private:

    std::string PUBKEY;
    std::string token;
    std::string WSS_URL;
    int heartbeat_interval;

    CURL *curl;
    CURLcode res;
    std::string readBuffer;

    ws_client uWS;

    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);

    void startHeartbeat();
public:

    void setPUBKEY(std::string pubkey){
        this->PUBKEY = pubkey;}
    void setToken(std::string token){
        this->token = token;}

    Wool(); // Constructor
    ~Wool(); // Destructor

    void connect_ws();

    void sendMsg(std::string msg, int64_t channelID, bool allowMention = true);
};// class Wool

class WoolHelper {
    public:
        static void setHeartbeatInterval(Wool& wool, int interval) {
            wool.heartbeat_interval = interval;
        }
};// class WoolHelper

#endif // WOOL_HPP