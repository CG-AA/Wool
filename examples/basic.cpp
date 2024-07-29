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
    Wool::Wool wool;
    try{
        nlohmann::json config = getConfig();
        std::string token = config["token"];
        int64_t channelID = 872341868149637211;
        wool.setToken(token);
        wool.setWssMessageHandler([](std::string message){
            SPDLOG_INFO("Received message: {}", message);
        });
        wool.setIntents(1 << 9);
        wool.connect_ws();
        nlohmann::json eg_components = nlohmann::json::parse(R"(
            {
                "content": "This is a message with components",
                "components": [
                    {
                        "type": 1,
                        "components": [
                            {
                                "type": 2,
                                "style": 1,
                                "label": "Button",
                                "custom_id": "button1"
                            }
                        ]
                    }
                ]
            }
        )");
        SPDLOG_INFO("Sending message...");
        wool.sendHTTP("/channels/" + std::to_string(channelID) + "/messages", "POST", eg_components.dump());

        wool.run();
    } catch (std::exception& e) {
        SPDLOG_ERROR("std::exception: {}", e.what());
    }
    SPDLOG_DEBUG("Exiting...");
    return 0;
}