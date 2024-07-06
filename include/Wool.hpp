#ifndef WOOL_HPP
#define WOOL_HPP

#include <curl/curl.h>

class Wool {
public:
    uWS::App app;

    void setPUBKEY(std::string pubkey){
        this->PUBKEY = pubkey;}
    void setToken(std::string token){
        this->token = token;}

    Wool(); // Constructor

    void setupSecureConnection();

    void run(); // Method to start the WebSocket server

private:
    std::string PUBKEY;
    std::string token;

    void setupRoutes();
};

#endif // WOOL_HPP