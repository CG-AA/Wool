#include <Wool/Wool.hpp>

int main() {
    Wool wool;
    std::string token = "your_bot's_pubkey";
    int64_t channelID = 1234567890; // Your channel ID
    wool.setToken();
    wool.sendMsg("Hello, World!", , true);
    wool.~Wool();
    return 0;
}