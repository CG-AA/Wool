#ifndef WOOL_VOICE_HPP
#define WOOL_VOICE_HPP

#include <string>
#include <vector>
#include <functional>
#include"Wool.hpp"

namespace Wool {

class Voice {
public:
	Voice(const std::string& guild_id, const std::string& channel_id);
	void connect();
	void disconnect();
    void setVoiceInputHandler(const std::function<void(const std::vector<uint8_t>&)>& handler){
        onVoiceInput = handler; }
    void setVoiceOutputHandler(const std::function<std::vector<uint8_t>()>& handler){
        onVoiceOutput = handler;    }

private:
	std::string endpoint;
	std::string token;
	std::string guild_id;
	std::string channel_id;
	std::function<void(const std::vector<uint8_t>&)> voiceInputHandler;
	std::function<std::vector<uint8_t>()> voiceOutputHandler;
};

} // namespace Wool

#endif // WOOL_VOICE_HPP