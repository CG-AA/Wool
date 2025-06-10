#include "Wool.hpp"
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <channel_id> <message>\n";
        return 1;
    }
    Wool::Message msg;
    msg.content = argv[2];
    try {
        Wool::Message res = Wool::Message::CreateMessage(std::stoull(argv[1]), msg);
        std::cout << "Sent message ID: " << res.id << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
