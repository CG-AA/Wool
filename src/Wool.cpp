#include "../include/Wool.hpp"
#include <spdlog/spdlog.h>
#include <chrono>
#include <thread>

// data received(void *contents) to string(std::string *userp)
//used by curl
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

void Wool::initMessageHandler(websocketpp::connection_hdl hdl, ws_client::message_ptr msg) {
    nlohmann::json message = nlohmann::json::parse(msg->get_payload());
    WoolHelper::setHeartbeatInterval(*this, message["d"]["heartbeat_interval"] * 0.9);
    SPDLOG_INFO("Heartbeat interval: {}", heartbeat_interval);
    std::thread heartbeatThread([this](){
        std::this_thread::sleep_for(std::chrono::milliseconds(heartbeat_interval) * std::rand()%100 / 100);
        while (true) {
            nlohmann::json heartbeat = {
                {"op", 1},
                {"d", 251}
            };
            WSpp.send(hdl, heartbeat.dump(), websocketpp::frame::opcode::text);
            std::this_thread::sleep_for(std::chrono::milliseconds(heartbeat_interval));
        }
    });
}


void Wool::connect_ws(){
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
    try {
        ws_client WSC;
        WSC.init_asio();
        //tlsv12 init
        WSC.set_tls_init_handler([](websocketpp::connection_hdl) {
            return websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12);
        });
        WSC.set_message_handler([&WSC](websocketpp::connection_hdl hdl, ws_client::message_ptr msg) {
            SPDLOG_INFO("Message from WS: {}", msg->get_payload());
        });
        WSC.set_open_handler([&WSC](websocketpp::connection_hdl hdl) {
            SPDLOG_INFO("Connected to Discord websocket :D");
        });
        websocketpp::lib::error_code ec;    // check ec to see if there were errors
        auto conn = WSC.get_connection(WSS_URL, ec);
        if (ec) {
            SPDLOG_ERROR("Could not create connection because: {}", ec.message());
            return;
        }
        WSC.connect(conn);
        WSC.run();
    } catch (websocketpp::exception const & e) {
        SPDLOG_ERROR("Websocketpp exception: {}", e.what());
    } catch (std::exception const & e) {
        SPDLOG_ERROR("std::exception: {}", e.what());
    }
    curl_slist_free_all(headers); // Free the header list
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
    
