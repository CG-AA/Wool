#include "../include/Wool.hpp"
#include <spdlog/spdlog.h>
#include <chrono>
#include <thread>

// data received(void *contents) to string(std::string *userp)
// used by curl
size_t Wool::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

Wool::Wool() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
}

Wool::~Wool() {
    closeWebSocket();
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
                    this->WSppC.send(hdl, heartbeat.dump(), websocketpp::frame::opcode::text);
                    this->ACK = false;
                    std::this_thread::sleep_for(std::chrono::milliseconds(heartbeat_interval));
                }
                this->WSppC.close(hdl, websocketpp::close::status::protocol_error, "Heartbeat ACK not received");
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
        //handle sequence manually (its faster than using the json lib)
        //about 80x faster
        std::string payload = msg->get_payload();
        size_t startPos = payload.find("\"s\":");
        if (startPos != std::string::npos) {
            startPos += 4; // Move past the key and the colon
            size_t endPos = payload.find_first_of(',', startPos);
            if (endPos != std::string::npos) {
                std::string sValueStr = payload.substr(startPos, endPos - startPos);
                // Convert to int or leave as string depending on your needs
                try {
                    int sValue = std::stoi(sValueStr);
                    this->LS = sValue; // Assuming LS is an int member variable for last sequence
                    SPDLOG_DEBUG("Updated LS to {}", sValue);
                } catch (const std::invalid_argument& e) {
                    SPDLOG_ERROR("Invalid argument: {}", e.what());
                } catch (const std::out_of_range& e) {
                    SPDLOG_ERROR("Out of range: {}", e.what());
                }
            }
        }
        // give message to user-defined handler
        onWssMessage(payload);
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
    WSppC.send(hdl, identify.dump(), websocketpp::frame::opcode::text);
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
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    CURLcode res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
        SPDLOG_ERROR("curl_easy_perform() failed: {}", curl_easy_strerror(res));
    } else {
        try {
            SPDLOG_DEBUG("Received response: {}", readBuffer);
            nlohmann::json response = nlohmann::json::parse(readBuffer);
            WSS_URL = response["url"];
            SPDLOG_INFO("gateway URL received: {}", WSS_URL);
        } catch (nlohmann::json::parse_error& e) {
            SPDLOG_ERROR("JSON parsing failed: {}", e.what());
        }
    }
    readBuffer.clear();
    curl_slist_free_all(headers);

    WSppC.init_asio();
    //tlsv12 init
    WSppC.set_tls_init_handler([](websocketpp::connection_hdl) {
        return websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12);
    });
    WSppC.set_message_handler([this](websocketpp::connection_hdl hdl, ws_client::message_ptr msg) {
        (this->*messageHandler)(hdl, msg);
    });
    WSppC.set_open_handler([](websocketpp::connection_hdl hdl) {
        SPDLOG_INFO("Connected to Discord websocket :D");
    });
    WSppC.set_close_handler([this](websocketpp::connection_hdl hdl) {
        SPDLOG_WARN("Disconnected from Discord websocket :(");
        reconnect_ws();
    });
    websocketpp::lib::error_code ec;    // check ec to see if there were errors
    auto conn = WSppC.get_connection(WSS_URL, ec);
    if (ec) {
        SPDLOG_ERROR("Could not create connection because: {}", ec.message());
        return;
    }
    WSppC.connect(conn);
    std::thread WSppC_Thread([this](){
        WSppC.run();
    });
    WSppC_Thread.detach();
}//connect_ws

void Wool::reconnect_ws(){
    try{
        SPDLOG_INFO("Reconnecting to websocket...");
        LS = 0;
        ACK = false;
        inited = false;
        messageHandler = &Wool::initMessageHandler;
        websocketpp::lib::error_code ec;    // check ec to see if there were errors
        auto conn = WSppC.get_connection(WSS_URL, ec);
        if (ec) {
            SPDLOG_ERROR("Could not create connection because: {}", ec.message());
            return;
        }
        WSppC.connect(conn);
    } catch (std::exception& e) {
        SPDLOG_ERROR("std::exception: {}", e.what());
    }
}

void Wool::sendHTTP(const std::string& path, const std::string& method, const std::string& data) {
    curl_easy_reset(curl);
    std::string url = "https://discord.com/api/v10" + path;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: Bot " + token).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    if (method == "POST") {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    SPDLOG_INFO("Sending message: {}", data);
    SPDLOG_INFO("Sending request to: {}", url);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        SPDLOG_ERROR("message failed to send: {}", curl_easy_strerror(res));
        SPDLOG_ERROR("reason: {}", readBuffer);
    } else {
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code >= 400) {
            SPDLOG_WARN("HTTP error {}: {}", http_code, readBuffer);
        } else {
            SPDLOG_INFO("message sent: {}", readBuffer);
        }
    }
    readBuffer.clear();
    curl_slist_free_all(headers);
}

void Wool::run() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this] { return stopFlag; });
}

void Wool::stop() {
    stopFlag = true;
    cv.notify_all();
}

void Wool::closeWebSocket() {
    WSppC.stop();
    WSppC.close(websocketpp::close::status::normal, "Closing connection");
    SPDLOG_INFO("WebSocket connection closed");
}