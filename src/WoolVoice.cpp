#include"../include/WoolVoice.hpp"

namespace Wool {
    Voice::Voice(const std::string& guild_id, const std::string& channel_id){
        : guild_id(guild_id), channel_id(channel_id) {
        }
    }

    void Voice::connect() {
        
        Wool::sendWss()
    }
        
        
}// namespace Wool