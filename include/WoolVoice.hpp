#ifndef WOOL_VOICE_HPP
#define WOOL_VOICE_HPP

#include <string>
#include <vector>
#include <functional>
#include <spdlog/spdlog.h>
#include <mutex>
#include"Wool.hpp"

namespace Wool {

class Voice {
public:
	Voice(const Wool::Wool& WoolINS , const std::string& guild_id, const std::string& channel_id, const bool deaf, const bool mute);
	void connect();
	void disconnect();
    void setVoiceInputHandler(const std::function<void(const std::vector<uint8_t>&)>& handler){
        onVoiceInput = handler; }
    void setVoiceOutputHandler(const std::function<std::vector<uint8_t>()>& handler){
        onVoiceOutput = handler;    }

private:
    std::mutex mtx;
    Wool::Wool WoolINS;
    bool deaf;
    bool mute;
	std::string endpoint;
	std::string token;
	std::string guild_id;
	std::string channel_id;
    std::string session_id;
    std::string user_id;
    std::string session_id;
	std::function<void(const std::vector<uint8_t>&)> voiceInputHandler;
	std::function<std::vector<uint8_t>()> voiceOutputHandler;

    void parseVoiceServerUpdate(const std::string& data);
    std::atomic<bool> VCSeUreceived{false};
    void parseVoiceStateUpdate(const std::string& data);
    std::atomic<bool> VCStUreceived{false};
};

} // namespace Wool

#endif // WOOL_VOICE_HPP