#include <nlohmann/json.hpp>
#include"../include/WoolVoice.hpp"

namespace {
    std::string getComponentString(std::string& data, const std::string& key) {
        std::string searchKey = "\"" + key + "\":\"";
        size_t startPos = data.find(searchKey);
        if (startPos != std::string::npos) {
            startPos += searchKey.length(); // Move past the key and the colon
            size_t endPos = data.find_first_of('\"', startPos);
            if (endPos != std::string::npos) {
                return data.substr(startPos, endPos - startPos);
            }
        }
        SPDLOG_ERROR("Key not found: {}", key);
        throw std::invalid_argument("Key not found");
    }
}

namespace Wool {
    Voice::Voice(Wool* WoolINS , const std::string& guild_id, const std::string& channel_id, const bool deaf, const bool mute)
        : guild_id(guild_id), channel_id(channel_id), deaf(deaf), mute(mute), WoolINS(WoolINS) {
            onVWSmsg = [this](websocketpp::connection_hdl hdl, std::string msg) {
                initVoiceWSmsgHandler(hdl, msg);
            };
    }

    void Voice::parseVoiceServerUpdate(std::string& data) {
        token = getComponentString(data, "token");
        endpoint = getComponentString(data, "endpoint");
        VCSeUreceived = true;
        cv.notify_one();
    }
    void Voice::parseVoiceStateUpdate(std::string& data) {
        session_id = getComponentString(data, "session_id");
        user_id = getComponentString(data, "user_id");
        VCStUreceived = true;
        cv.notify_one();
    }

    void Voice::connect() {
        std::unique_lock<std::mutex> lock(mtx);
        WoolINS->WSready_cv.wait(lock, [this] { return WoolINS->ready.load(); });
        WoolINS->onVoiceUpdate = [this](std::string data) {
            std::string tVal = getComponentString(data, "t");
            try {
                if (tVal == "VOICE_SERVER_UPDATE" && !VCSeUreceived) {
                    parseVoiceServerUpdate(data);
                    SPDLOG_INFO("Received voice server update");
                } else if (tVal == "VOICE_STATE_UPDATE" && !VCStUreceived) {
                    parseVoiceStateUpdate(data);
                    SPDLOG_INFO("Received voice state update");
                }
                return;
            } catch (const std::invalid_argument& e) {
                SPDLOG_ERROR("Invalid argument: {}", e.what());
            } catch (const std::out_of_range& e) {
                SPDLOG_ERROR("Out of range: {}", e.what());
            }
        };
        SPDLOG_INFO("Sending voice state update...");
        //Gateway Voice State Update
        WoolINS->sendWss("{\"op\":4,\"d\":{\"guild_id\":\"" + guild_id + "\",\"channel_id\":\"" + channel_id + "\",\"self_mute\":false,\"self_deaf\":false}}");
        cv.wait(lock, [this] { return VCSeUreceived && VCStUreceived; });
        WoolINS->onVoiceUpdate = nullptr;
        SPDLOG_INFO("Connecting to voice WS...");
        connectVoiceWS();
    }
        
    void Voice::connectVoiceWS() {
        voiceWS.init_asio();
        voiceWS.set_tls_init_handler([](websocketpp::connection_hdl) {
            return websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12);
        });
        voiceWS.set_open_handler([this](websocketpp::connection_hdl hdl) {
            SPDLOG_INFO("Sending voice identify...");
            voiceWS.send(hdl, "{\"op\":0,\"d\":{\"server_id\":\"" + guild_id + "\",\"user_id\":\"" + user_id + "\",\"session_id\":\"" + session_id + "\",\"token\":\"" + token + "\"}}", websocketpp::frame::opcode::text);
        });
        voiceWS.set_message_handler([this](websocketpp::connection_hdl hdl, ws_client::message_ptr msg) {
            onVWSmsg(hdl, msg->get_payload());
        });
        websocketpp::lib::error_code ec;
        std::string VCurl = "wss://" + endpoint + "?v=4";
        SPDLOG_INFO("Connecting to: {}", VCurl);
        ws_client::connection_ptr con = voiceWS.get_connection(VCurl, ec);
        if (ec) {
            SPDLOG_ERROR("Could not create connection: {}", ec.message());
            return;
        }
        voiceWS.connect(con);
        std::thread voiceWS_thread([this](){
            voiceWS.run();
        });
        voiceWS_thread.detach();
    }

    void Voice::initVoiceWSmsgHandler(websocketpp::connection_hdl hdl, std::string msg) {
        nlohmann::json j = nlohmann::json::parse(msg);
        if (j["op"] == 2){
            ssrc = j["d"]["ssrc"];
            ip = j["d"]["ip"];
            port = j["d"]["port"];
            std::vector <std::string> modes = j["d"]["modes"];
            if(std::find(modes.begin(), modes.end(), encryptionMode) != modes.end()){
                SPDLOG_INFO("Encryption mode: {}", encryptionMode);
                ready = true;
            }else{
                SPDLOG_ERROR("Unsupported encryption mode: {}", encryptionMode);
                SPDLOG_ERROR("Supported encryption modes: {}", j["d"]["modes"].dump());
                return;
            }
        }else if (j["op"] == 8){
            heartbeat_interval = j["d"]["heartbeat_interval"];
            hello = true;
        }else if (j["op"] == 4){
            secretKey = j["d"]["secret_key"];
            secretKeyReceived = true;
        }
        if (ready && hello && secretKeyReceived) {
            SPDLOG_INFO("Voice connection established");
            onVWSmsg = [this](websocketpp::connection_hdl hdl, std::string msg) {
                generalVoiceWSmsgHandler(hdl, msg);
            };
            return;
        }
        if (ready && hello && !secretKeyReceived) {
            voiceWS.send(hdl, R"({"op":1,"d":{"protocol":"udp","data":{"address":")" + ip + R"(","port":)" + std::to_string(port) + R"(,"mode":")" + encryptionMode + R"("}}})", websocketpp::frame::opcode::text);

            ACK = true;
            nonce = std::chrono::system_clock::now();
            std::thread heartbeatThread([this, hdl](){
                while (ACK) {
                    const std::string heartbeat = R"({"op":3,"d":)" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - nonce).count()) + "}";
                    voiceWS.send(hdl, heartbeat, websocketpp::frame::opcode::text);
                    ACK = false;
                    std::this_thread::sleep_for(std::chrono::milliseconds(heartbeat_interval));
                }
                this->voiceWS.close(hdl, websocketpp::close::status::protocol_error, "Heartbeat ACK not received");
                SPDLOG_WARN("Didn't receive heartbeat ACK, attempting to reconnect...");
                this->reconnectVoiceWS();
            });
            heartbeatThread.detach();
            return;
        }
    }

    void Voice::reconnectVoiceWS() {
        voiceWS.stop();
        connectVoiceWS();
    }

    void Voice::generalVoiceWSmsgHandler(websocketpp::connection_hdl hdl, std::string message) {
        if (message.find(R"("op":6))") != std::string::npos) {
            ACK = true;
            SPDLOG_DEBUG("Heartbeat ACK received");
            return;
        }
        SPDLOG_INFO("Received message: {}", message);
    }
    
}// namespace Wool