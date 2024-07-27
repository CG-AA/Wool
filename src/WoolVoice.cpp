#include"../include/WoolVoice.hpp"

namespace {
    std::string getComponentString(const std::string& data, const std::string& key) {
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
    Voice::Voice(const std::string& guild_id, const std::string& channel_id){
        : guild_id(guild_id), channel_id(channel_id) {
        }
    }

    void Voice::parseVoiceServerUpdate(const std::string& data) {
        token = getComponentString(data, "token");
        endpoint = getComponentString(data, "endpoint");
        VCSeUreceived = true;
    }
    void Voice::parseVoiceStateUpdate(const std::string& data) {
        // Parse the data and set the token
        // Set VCStUreceived to true
    }

    void Voice::connect() {
        Wool::OnVoiceUpdate = [this](std::string data) {
            std::string tVal = getComponentString(data, "t");
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
    }
        
        
}// namespace Wool