#include <Wool/Wool.hpp>
#include <nlohmann/json.hpp>
#include <fstream>

nlohmann::json getConfig(){
    std::ifstream fileStream("config.json");
    nlohmann::json config = nlohmann::json::parse(fileStream);
    return config;
}

int main() {
    Wool wool;
    nlohmann::json config = getConfig();
    std::string token = config["token"];
    int64_t channelID = 872341868149637211; // Your channel ID
    wool.setToken(token);
    wool.connect_ws();
    wool.sendMsg("Hello, World!", channelID, true);
    wool.~Wool();
    return 0;
}