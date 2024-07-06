#include "../include/Wool.hpp"
#include <spdlog/spdlog.h>

// data received(void *contents) to string(std::string *userp)
//used by curl
size_t Wool::WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    // Cast userp back to Wool* to access instance data
    Wool* instance = static_cast<Wool*>(userp);
    size_t realSize = size * nmemb;
    instance->readBuffer.append((char*)contents, realSize);
    return realSize;
}

Wool::Wool() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
}

Wool::~Wool() {
    curl_easy_cleanup(curl);
    curl_global_cleanup();
}

void Wool::sendMsg(std::string msg, int64_t channelID, bool allowMention) {
    curl_easy_reset(curl);
    std::string url = "https://discord.com/api/v10/channels/" + std::to_string(channelID) + "/messages";
    nlohmann::json allowed_mentions;
    if (allowMention) {
        allowed_mentions = {{"parse", {"users", "roles", "everyone"}}};
    } else {
        allowed_mentions = {{"parse", {}}};
    }

    nlohmann::json data = {
        {"content", msg},
        {"allowed_mentions", allowed_mentions}
    };
    std::string dataStr = data.dump();
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, ("Authorization: Bot " + token).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, dataStr.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

    // Perform the request, res will get the return code
    res = curl_easy_perform(curl);
    // Check for errors
    if(res != CURLE_OK) {
        SPDLOG_ERROR("curl_easy_perform() failed: {}", curl_easy_strerror(res));
    } else {
        // You can use the response_string for further processing
        SPDLOG_INFO("Response: {}", readBuffer);
    }
    readBuffer.clear();
    curl_slist_free_all(headers);
}
    
