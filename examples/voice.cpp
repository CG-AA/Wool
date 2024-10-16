#include <Wool/Wool.hpp>
#include <Wool/WoolVoice.hpp>
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
    Wool::Wool wool;
    try{
        nlohmann::json config = getConfig();
        std::string token = config["token"];
        int64_t channelID = 872341868149637211;
        wool.setToken(token);
        wool.setWssMessageHandler([](std::string message){
            SPDLOG_INFO("Received message: {}", message);
        });
        wool.setIntents(1 << 9 | 1 << 7);
        SPDLOG_INFO("Connecting to the gateway...");
        wool.connect_ws();
        Wool::Voice voice(&wool, "745136869154750473", "1128985878467989574", false, false);
        voice.connect();
        wool.run();
    } catch (std::exception& e) {
        SPDLOG_ERROR("std::exception: {}", e.what());
    }
    SPDLOG_DEBUG("Exiting...");
    return 0;
}