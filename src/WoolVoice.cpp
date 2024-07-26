#include"../include/WoolVoice.hpp"

namespace Wool {
    Voice::Voice(const std::string& guild_id, const std::string& channel_id){
        : guild_id(guild_id), channel_id(channel_id) {
        }
    }

    void Voice::connect() {
        Wool::OnVoiceUpdate = [this](std::string data) {
            size_t startPos = data.find("\"t\":");
            if (startPos != std::string::npos) {
                startPos += 4; // Move past the key and the colon
                size_t endPos = data.find_first_of(',', startPos);
                if (endPos != std::string::npos) {
                    std::string sValueStr = data.substr(startPos, endPos - startPos);
                    // Convert to int or leave as string depending on your needs
                    try {
                        if (sValueStr == "VOICE_SERVER_UPDATE") {
                            parseVoiceServerUpdate(data);
                        } else if (sValueStr == "VOICE_STATE_UPDATE") {
                            parseVoiceStateUpdate(data);
                        }
                    } catch (const std::invalid_argument& e) {
                        SPDLOG_ERROR("Invalid argument: {}", e.what());
                    } catch (const std::out_of_range& e) {
                        SPDLOG_ERROR("Out of range: {}", e.what());
                    }
                }
            }
        };
    }
        
        
}// namespace Wool