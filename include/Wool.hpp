#ifndef WOOL_HPP
#define WOOL_HPP

#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <mutex>

typedef websocketpp::client<websocketpp::config::asio_tls_client> ws_client;

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
    CURLcode res;
    std::string readBuffer;

    ws_client WSpp;

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
    class Message {
    public:
        struct embed{
            std::string title;
            std::string type;
            std::string description;
            std::string url;
            std::string timestamp;
            int32_t color;
        };
        struct allowed_mentions{
            
        };
        struct message_reference{
            
        };
        struct components{
            
        };
        struct poll{
            
        };
        int64_t channelID;
        std::string content;
        // nonce (todo)
        bool tts = false;
        std::vector<nlohmann::json> embeds;
        nlohmann::json allowed_mentions;
        nlohmann::json message_reference;
        nlohmann::json components;
        std::vector<int64_t> sticker_ids;//I don't know what this is
        // files and payload_json (todo)
        // attachments (todo)
        int64_t flags = 0;
        // enforce_nonce (todo)
    };
    
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

    void sendMsg(std::string msg, int64_t channelID, bool allowMention = true);
};// class Wool

#endif // WOOL_HPP