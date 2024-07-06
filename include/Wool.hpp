#ifndef WOOL_HPP
#define WOOL_HPP

#include <curl/curl.h>
#include <nlohmann/json.hpp>

class Wool {
private:
    std::string PUBKEY;
    std::string token;

    CURL *curl;
    CURLcode res;
    std::string readBuffer;

    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
public:

    void setPUBKEY(std::string pubkey){
        this->PUBKEY = pubkey;}
    void setToken(std::string token){
        this->token = token;}

    Wool(); // Constructor
    ~Wool(); // Destructor

    sendMsg(std::string msg, int64_t channelID, bool allowMention = true);
};

#endif // WOOL_HPP