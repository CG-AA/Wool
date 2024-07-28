#ifndef WOOL_VOICE_HPP
#define WOOL_VOICE_HPP

#include <string>
#include <vector>
#include <functional>
#include <spdlog/spdlog.h>
#include <condition_variable>
#include <atomic>
#include <mutex>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>
#include"Wool.hpp"

typedef websocketpp::client<websocketpp::config::asio_tls_client> ws_client;

namespace Wool {

class Voice {
public:
	Voice(Wool* WoolINS , const std::string& guild_id, const std::string& channel_id, const bool deaf, const bool mute);
	void connect();
	void disconnect();
    void setVoiceInputHandler(const std::function<void(const std::vector<uint8_t>&)>& handler){
        onVoiceInput = handler; }
    void setVoiceOutputHandler(const std::function<std::vector<uint8_t>()>& handler){
        onVoiceOutput = handler;    }

private:
    std::mutex mtx;
    std::condition_variable cv;
    std::unique_ptr<Wool> WoolINS;
    ws_client voiceWS;
    bool deaf;
    bool mute;
	std::string endpoint;
	std::string token;
	std::string guild_id;
	std::string channel_id;
    std::string user_id;
    std::string session_id;
    int ssrc;
    std::string ip;
    int port;
    int heartbeat_interval;
	std::function<void(const std::vector<uint8_t>&)> onVoiceInput;
	std::function<std::vector<uint8_t>()> onVoiceOutput;

    void parseVoiceServerUpdate(std::string& data);
    std::atomic<bool> VCSeUreceived{false};
    void parseVoiceStateUpdate(std::string& data);
    std::atomic<bool> VCStUreceived{false};

    void connectVoiceWS();

    void initVoiceWSmsgHandler(std::string msg);
    void generalVoiceWSmsgHandler(std::string msg);
    std::function<void(std::string)> onVWSmsg = initVoiceWSmsgHandler;

};

} // namespace Wool

#endif // WOOL_VOICE_HPP