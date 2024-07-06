#include "../include/Wool.hpp"

// data received(void *contents) to string(std::string *userp)
//used by curl
static size_t Wool::WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

Wool::Wool() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://discord.com/api/v10");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
    }
}

Wool::~Wool() {
    curl_easy_cleanup(curl);
    curl_global_cleanup();
}

Wool::run() {
}