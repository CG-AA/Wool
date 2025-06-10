#include "Wool.hpp"
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <channel_id>\n";
        return 1;
    }
    const Wool::Snowflake channel_id = std::stoull(argv[1]);
    Wool::Message msg;
    msg.content = "Test message from automated test";
    try {
        Wool::Message res = Wool::Message::CreateMessage(channel_id, msg);
        std::cout << "Sent message ID: " << res.id << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error sending message: " << e.what() << "\n";
        return 1;
    }
}
