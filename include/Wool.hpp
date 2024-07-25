#ifndef WOOL_HPP
#define WOOL_HPP

#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <mutex>

typedef websocketpp::client<websocketpp::config::asio_tls_client> ws_client;
typedef websocketpp::server<websocketpp::config::asio_tls> ws_server;

class Wool {
private:
    // std::string PUBKEY = ""; //might not be needed
    std::string token = "";
    std::string APPID = "";
    std::string WSS_URL;
    int heartbeat_interval;
    std::atomic<int> LS{0};//last sequence (s in message)
    std::atomic<bool> ACK{false};//heartbeat ACK
    bool inited = false;

    CURL *curl;
    std::string readBuffer;

    ws_client WSppC;
    

    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);

    // messageHandlers
    void (Wool::*messageHandler)(websocketpp::connection_hdl, ws_client::message_ptr) = &Wool::initMessageHandler;
    void initMessageHandler(websocketpp::connection_hdl hdl, ws_client::message_ptr msg);
    void generalMessageHandler(websocketpp::connection_hdl hdl, ws_client::message_ptr msg);

    // user-defined message handlers (string)
    std::function<void(std::string)> onWssMessage;

    void sendIdentify(websocketpp::connection_hdl hdl);

    void reconnect_ws();
public:
    
    void setPUBKEY(std::string pubkey){
        this->PUBKEY = pubkey;}
    void setToken(std::string token){
        this->token = token;}
    
    void setWssMessageHandler(const std::function<void(std::string)> &handler){
        onWssMessage = handler;
    }

    Wool(); // Constructor
    ~Wool(); // Destructor

    void connect_ws();

    /**
     *  check the Discord developer documentation for the usage
     *  example: 
     *  sendHTTP("/channels/1234567890/messages", "POST", "{\"content\":\"Hello, World!\"}");
     */ 
    void sendHTTP(std::string path, std::string method, std::string data);
};// class Wool

#endif // WOOL_HPP