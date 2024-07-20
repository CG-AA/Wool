#include <Wool/Wool.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <spdlog/spdlog.h>

nlohmann::json getConfig(){
    nlohmann::json config;
    try {
        std::ifstream fileStream("config.json");
        config = nlohmann::json::parse(fileStream);
    } catch (nlohmann::json::parse_error& e) {
        SPDLOG_ERROR("failed to parse config.json: {}", e.what());
    }
    return config;
}


int main() {
    try{
        Wool wool;
        nlohmann::json config = getConfig();
        std::string token = config["token"];
        int64_t channelID = 872341868149637211; // Your channel ID
        wool.setToken(token);
        wool.connect_ws();
    } catch (std::exception& e) {
        SPDLOG_ERROR("std::exception: {}", e.what());
    }
    return 0;
}