#ifndef WOOL_HPP
#define WOOL_HPP

#include <uWebSockets/App.h>

class Wool {
public:
    std::string PUBKEY;
    std::string token;
    short port = 45900;

    Wool(); // Constructor
    ~Wool(); // Destructor

    void run(); // Method to start the WebSocket server

private:
    void onConnection(uWS::WebSocket<false, true>* ws, uWS::HttpRequest* req);
    void onMessage(uWS::WebSocket<false, true>* ws, std::string_view message, uWS::OpCode opCode);
    void onClose(uWS::WebSocket<false, true>* ws, int code, std::string_view message);
};

#endif // WOOL_HPP