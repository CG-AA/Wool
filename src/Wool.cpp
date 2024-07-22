#include "../include/Wool.hpp"
#include <spdlog/spdlog.h>
#include <chrono>
#include <thread>

// data received(void *contents) to string(std::string *userp)
// used by curl
size_t Wool::WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    // Cast userp back to Wool* to access instance data
    Wool* instance = static_cast<Wool*>(userp);
    size_t realSize = size * nmemb;
    instance->readBuffer.append((char*)contents, realSize);
    return realSize;
}

Wool::Wool() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
}

Wool::~Wool() {
    curl_global_cleanup();
}

/**
 * create heartbeat thread and switch to general message handler
 */
void Wool::initMessageHandler(websocketpp::connection_hdl hdl, ws_client::message_ptr msg) {
    try{
        nlohmann::json message = nlohmann::json::parse(msg->get_payload());
        SPDLOG_DEBUG("Received message: {}", message.dump());
        if(!inited){    //handle HELLO message
            inited = true;
            heartbeat_interval = int(message["d"]["heartbeat_interval"]) * 0.9;
            SPDLOG_INFO("Heartbeat interval: {}", heartbeat_interval);
            ACK = true;
            std::thread heartbeatThread([this, hdl](){
                std::this_thread::sleep_for(std::chrono::milliseconds(heartbeat_interval) * (std::rand()%100) / 100);
                while (this->ACK) {
                    nlohmann::json heartbeat = {
                        {"op", 1},
                        {"d", int(this->LS)}
                    };
                    this->WSpp.send(hdl, heartbeat.dump(), websocketpp::frame::opcode::text);
                    this->ACK = false;
                    std::this_thread::sleep_for(std::chrono::milliseconds(heartbeat_interval));
                }
                this->WSpp.close(hdl, websocketpp::close::status::protocol_error, "Heartbeat ACK not received");
                SPDLOG_WARN("Didn't receive heartbeat ACK, attempting to reconnect...");
                this->reconnect_ws();
            });
            heartbeatThread.detach();
            SPDLOG_INFO("Heartbeat thread started");
            sendIdentify(hdl);
        } else {    //handle READY message
            if(message["t"].get<std::string>() != "READY") {
                return;
            }
            APPID = message["d"]["application"]["id"];
            SPDLOG_INFO("Application ID: {}", APPID);
            SPDLOG_INFO("Switching to general message handler");
            messageHandler = &Wool::generalMessageHandler;
        }
    } catch (nlohmann::json::parse_error& e) {
        SPDLOG_ERROR("JSON parsing failed: {}", e.what());
    } catch (std::exception& e) {
        SPDLOG_ERROR("std::exception: {}", e.what());
    }
}

void Wool::generalMessageHandler(websocketpp::connection_hdl hdl, ws_client::message_ptr msg) {
    try{
        // handle heartbeat ACK
        if (msg->get_payload() == "{\"t\":null,\"s\":null,\"op\":11,\"d\":null}") {
            ACK = true;
            SPDLOG_DEBUG("Heartbeat ACK received");
            return;
        }
        nlohmann::json message = nlohmann::json::parse(msg->get_payload());
        SPDLOG_INFO("Received message: {}", msg->get_payload());
        // update last sequence
        if (!message["s"].empty())this->LS = int(message["s"]);
        // give message to user-defined handler
        onWssMessage(msg->get_payload());
    } catch (nlohmann::json::parse_error& e) {
        SPDLOG_ERROR("JSON parsing failed: {}", e.what());
    } catch (std::exception& e) {
        SPDLOG_ERROR("std::exception: {}", e.what());
    }
}

void Wool::sendIdentify(websocketpp::connection_hdl hdl) {
    int intents = (1 << 9);
    nlohmann::json identify = {
        {"op", 2},
        {"d", {
            {"token", token},
            {"intents", intents},
            {"properties", {
                {"$os", "linux"},
                {"$browser", "Wool"},
                {"$device", "Wool"}
            }}
        }}
    };
    SPDLOG_INFO("Sending identify message: {}", identify.dump());
    WSpp.send(hdl, identify.dump(), websocketpp::frame::opcode::text);
}

void Wool::connect_ws(){
    if(token.empty()){
        SPDLOG_ERROR("Token is empty");
        throw std::runtime_error("Token is empty");
        return;
    }
    if(!onWssMessage){
        SPDLOG_ERROR("onWssMessage is not set");
        throw std::runtime_error("onWssMessage is not set");
        return;
    }
    SPDLOG_INFO("Connecting to websocket...");
    SPDLOG_INFO("getting gateway URL...");
    curl_easy_reset(curl);
    std::string url = "https://discord.com/api/v10/gateway/bot";
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, ("Authorization: Bot " + token).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

    res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
        SPDLOG_ERROR("curl_easy_perform() failed: {}", curl_easy_strerror(res));
    } else {
        try {
            nlohmann::json response = nlohmann::json::parse(readBuffer);
            WSS_URL = response["url"];
            SPDLOG_INFO("gateway URL received: {}", WSS_URL);
        } catch (nlohmann::json::parse_error& e) {
            SPDLOG_ERROR("JSON parsing failed: {}", e.what());
        }
    }
    readBuffer.clear();
    curl_slist_free_all(headers);

    WSpp.set_access_channels(websocketpp::log::alevel::all);

    WSpp.init_asio();
    //tlsv12 init
    WSpp.set_tls_init_handler([](websocketpp::connection_hdl) {
        return websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12);
    });
    WSpp.set_message_handler([this](websocketpp::connection_hdl hdl, ws_client::message_ptr msg) {
        (this->*messageHandler)(hdl, msg);
    });
    WSpp.set_open_handler([](websocketpp::connection_hdl hdl) {
        SPDLOG_INFO("Connected to Discord websocket :D");
    });
    websocketpp::lib::error_code ec;    // check ec to see if there were errors
    auto conn = WSpp.get_connection(WSS_URL, ec);
    if (ec) {
        SPDLOG_ERROR("Could not create connection because: {}", ec.message());
        return;
    }
    WSpp.connect(conn);
    WSpp.run();
}//connect_ws

void Wool::reconnect_ws(){
    try{
        SPDLOG_INFO("Reconnecting to websocket...");
        LS = 0;
        ACK = false;
        inited = false;
        messageHandler = &Wool::initMessageHandler;
        websocketpp::lib::error_code ec;    // check ec to see if there were errors
        auto conn = WSpp.get_connection(WSS_URL, ec);
        if (ec) {
            SPDLOG_ERROR("Could not create connection because: {}", ec.message());
            return;
        }
        WSpp.connect(conn);
    } catch (std::exception& e) {
        SPDLOG_ERROR("std::exception: {}", e.what());
    }
}

void Wool::sendMsg(std::string msg, int64_t channelID, bool allowMention) {
    curl_easy_reset(curl);
    std::string url = "https://discord.com/api/v10/channels/" + std::to_string(channelID) + "/messages";
    nlohmann::json allowed_mentions;
    if (allowMention) {
        allowed_mentions = {{"parse", {"users", "roles", "everyone"}}};
    } else {
        allowed_mentions = {{"parse", {}}};
    }

    nlohmann::json data = {
        {"content", msg},
        {"allowed_mentions", allowed_mentions}
    };
    std::string dataStr = data.dump();
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, ("Authorization: Bot " + token).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, dataStr.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

    // Perform the request, res will get the return code
    res = curl_easy_perform(curl);
    // Check for errors
    if(res != CURLE_OK) {
        SPDLOG_ERROR("curl_easy_perform() failed: {}", curl_easy_strerror(res));
    } else {
        SPDLOG_INFO("message sent: {}", readBuffer);
    }
    readBuffer.clear();
    curl_slist_free_all(headers);
}
    
