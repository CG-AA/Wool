#include <nlohmann/json.hpp>
#include"../include/WoolVoice.hpp"

namespace {
    std::string getComponentString(std::string& data, const std::string& key) {
        size_t startPos = data.find(key+"\":\"");
        if (startPos != std::string::npos) {
            startPos += 4; // Move past the key and the colon
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
        WoolINS->onVoiceUpdate = [this](std::string data) {
            std::string tVal = getComponentString(data, "t");
            try {
                if (tVal == "VOICE_SERVER_UPDATE" && !VCSeUreceived) {
                    parseVoiceServerUpdate(data);
                } else if (tVal == "VOICE_STATE_UPDATE" && !VCStUreceived) {
                    parseVoiceStateUpdate(data);
                }
                return;
            } catch (const std::invalid_argument& e) {
                SPDLOG_ERROR("Invalid argument: {}", e.what());
            } catch (const std::out_of_range& e) {
                SPDLOG_ERROR("Out of range: {}", e.what());
            }
        };
        //Gateway Voice State Update
        WoolINS->sendWss("{\"op\":4,\"d\":{\"guild_id\":\"" + guild_id + "\",\"channel_id\":\"" + channel_id + "\",\"self_mute\":false,\"self_deaf\":false}}");
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return VCSeUreceived && VCStUreceived; });
        WoolINS->onVoiceUpdate = [this](std::string data) {
            nlohmann::json j = nlohmann::json::parse(data);
            if (j["op"] == 2){
                ssrc = j["d"]["ssrc"];
                ip = j["d"]["ip"];
                port = j["d"]["port"];
                if(j["d"]["mode"].contains("xsalsa20_poly1305")){
                    SPDLOG_INFO("Encryption mode: xsalsa20_poly1305");
                }else{
                    SPDLOG_ERROR("Unsupported encryption mode");
                    return;
                }
            }
        };
        //Voice Identify Payload
        WoolINS->sendWss("{\"op\":0,\"d\":{\"server_id\":\"" + guild_id + "\",\"user_id\":\"" + user_id + "\",\"session_id\":\"" + session_id + "\",\"token\":\"" + token + "\"}}");


    }
        
        
}// namespace Wool