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
    }
}

namespace Wool {
    Voice::Voice(const Wool::Wool& WoolINS , const std::string& guild_id, const std::string& channel_id, const bool deaf, const bool mute)
        : guild_id(guild_id), channel_id(channel_id), deaf(deaf), mute(mute), WoolINS(WoolINS) {
    }

    void Voice::parseVoiceServerUpdate(const std::string& data) {
        token = getComponentString(data, "token");
        endpoint = getComponentString(data, "endpoint");
        VCSeUreceived = true;
        cv.notify_one();
    }
    void Voice::parseVoiceStateUpdate(const std::string& data) {
        session_id = getComponentString(data, "session_id");
        user_id = getComponentString(data, "user_id");
        VCStUreceived = true;
        cv.notify_one();
    }

    void Voice::connect() {
        WoolINS->OnVoiceUpdate = [this](std::string data) {
            std::string tVal = this->getComponentString(data, "t");
            try {
                if (tVal == "VOICE_SERVER_UPDATE") {
                    parseVoiceServerUpdate(data);
                } else if (tVal == "VOICE_STATE_UPDATE") {
                    parseVoiceStateUpdate(data);
                }
            } catch (const std::invalid_argument& e) {
                SPDLOG_ERROR("Invalid argument: {}", e.what());
            } catch (const std::out_of_range& e) {
                SPDLOG_ERROR("Out of range: {}", e.what());
            }
        };
        WoolINS.sendWsMessage("{\"op\":0,\"d\":{\"server_id\":\"" + guild_id + "\",\"channel_id\":\"" + channel_id + "\",\"self_mute\":false,\"self_deaf\":false}}");
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return VCSeUreceived && VCStUreceived; });
    }
        
        
}// namespace Wool