#include "../include/Wool.hpp"

// data received(void *contents) to string(std::string *userp)
//used by curl
static size_t Wool::WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

Wool::Wool() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
}

Wool::~Wool() {
    curl_easy_cleanup(curl);
    curl_global_cleanup();
}

Wool::sendMsg(std::string msg, int64_t channelID, bool allowMention) {
    curl_easy_reset(curl);
    std::string url = "https://discord.com/api/v10/channels/" + std::to_string(channelID) + "/messages";
    nlohmann::json data = {
        {"content", msg},
        {"allowed_mentions", {
            {"parse", allowMention ? {"users", "roles", "everyone"} : {}}
        }}
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
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    // Perform the request, res will get the return code
    res = curl_easy_perform(curl);
    // Check for errors
    if(res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    } else {
        // You can use the response_string for further processing
        std::cout << "Response from Discord: " << response_string << std::endl;
    }
    readBuffer.clear();
    curl_slist_free_all(headers);
}
    
