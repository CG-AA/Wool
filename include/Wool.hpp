#ifndef WOOL_HPP
#define WOOL_HPP

#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <mutex>

typedef websocketpp::client<websocketpp::config::asio_tls_client> ws_client;

class Wool {
friend class Voice;
private:
    std::mutex mtx;//block the main thread
    std::condition_variable cv;//unlocks the main thread(stop)
    std::atomic<bool> stopFlag{false};//stop the main thread

    // std::string PUBKEY = ""; //might not be needed
    std::string token = "";// bot token(found in the Discord developer portal)
    std::string APPID = "";// auto filled by the library
    std::string WSS_URL;// gateway URL
    int heartbeat_interval;//heartbeat interval (ms)
    std::atomic<int> LS{0};//last sequence (s in message)
    std::atomic<bool> ACK{false};//heartbeat ACK
    std::atomic<bool> inited{false};//set to true after HELLO message
    std::atomic<bool> ready{false};//set to true after READY message

    CURL *curl;//for HTTP requests
    std::string readBuffer;//for HTTP requests

    ws_client WSppC;//websocketpp client
    
    websocketpp::connection_hdl hdl;//websocketpp connection handle

    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);//used by curl

    // messageHandlers
    void (Wool::*messageHandler)(websocketpp::connection_hdl, ws_client::message_ptr) = &Wool::initMessageHandler;
    void initMessageHandler(websocketpp::connection_hdl hdl, ws_client::message_ptr msg);//set up the heartbeat thread and send identify
    void generalMessageHandler(websocketpp::connection_hdl hdl, ws_client::message_ptr msg);//receive heartbeat ACK and give other messages to the user

    // user-defined message handlers (string)
    std::function<void(std::string)> onWssMessage;

    void sendIdentify(websocketpp::connection_hdl hdl);//send identify message(used in initMessageHandler)

    void reconnect_ws();//reconnect to the gateway

    std::function<void(const std::vector<uint8_t>&)> onVoiceInput;
    std::function<std::vector<uint8_t>()> onVoiceOutput;
public:
    
    // void setPUBKEY(std::string pubkey){
        // this->PUBKEY = pubkey;}
    
    //set bot token(found in the Discord developer portal)
    void setToken(std::string token){
        this->token = token;}
    /**
     * message handler for websocket messages
     * @param handler: the function that will be called when a message is received
     */
    void setWssMessageHandler(const std::function<void(std::string)> &handler){
        onWssMessage = handler;
    }

    Wool(); // Constructor
    ~Wool(); // Destructor(includes cleanup)

    void connect_ws();//connect to the gateway

    void sendWss(const std::string& message);//send a text format message to the gateway
    void sendWss(const std::string& message, websocketpp::frame::opcode::value opcode);//send a message to the gateway with a specific gateway opcode

    /**
     *  check the Discord developer documentation for the usage
     *  example: 
     *  sendHTTP("/channels/1234567890/messages", "POST", "{\"content\":\"Hello, World!\"}");
     */ 
    std::string sendHTTP(const std::string& path, const std::string& method, const std::string& data);



    void run();
    void stop();
};// class Wool

#endif // WOOL_HPP