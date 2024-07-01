#ifndef WOOL_HPP
#define WOOL_HPP

#include <uWebSockets/App.h>

class Wool {
public:
    uWS::App app;

    setPUBKEY(std::string pubkey){
        this->PUBKEY = pubkey;}
    setToken(std::string token){
        this->token = token;}
    setPort(short port){
        this->port = port;}

    Wool(); // Constructor

    void run(); // Method to start the WebSocket server

private:
    std::string PUBKEY;
    std::string token;
    short port = 45900;

    void setupRoutes();
};

#endif // WOOL_HPP