#ifndef WOOL_HPP
#define WOOL_HPP

#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <mutex>

typedef websocketpp::client<websocketpp::config::asio_tls_client> ws_client;

class Wool {
friend class WoolHelper;
private:
    std::string PUBKEY = ""; // wait for implent
    std::string token = "";
    int APPID = 0;
    std::string WSS_URL;
    int heartbeat_interval;
    std::atomic<int> LS{0};//last sequence (s in message)
    std::atomic<bool> ACK{false};//heartbeat ACK
    bool inited = false;

    CURL *curl;
    CURLcode res;
    std::string readBuffer;

    ws_client WSpp;

    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);

    // messageHandlers
    void (Wool::*messageHandler)(websocketpp::connection_hdl, ws_client::message_ptr) = &Wool::initMessageHandler;
    void initMessageHandler(websocketpp::connection_hdl hdl, ws_client::message_ptr msg);
    void generalMessageHandler(websocketpp::connection_hdl hdl, ws_client::message_ptr msg);

    void sendIdentify(websocketpp::connection_hdl hdl);

    void reconnect_ws();
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

#endif // WOOL_HPP