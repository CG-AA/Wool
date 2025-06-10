#pragma once

#include <string>
#include <iostream>

namespace Wool {

// Basic objects used by the Wool API
class Message {
public:
    std::string content;
};

class Channel {
public:
    explicit Channel(std::string id) : id(std::move(id)) {}
    std::string id;
};

class User {
public:
    explicit User(std::string id) : id(std::move(id)) {}
    std::string id;
};

// Minimal interface for stage0
class Wool {
public:
    // Send a plain text message to a channel
    static Message send_message(const Channel &channel, const std::string &text) {
        std::cout << "[send_message] channel=" << channel.id << " text=" << text << std::endl;
        Message m; m.content = text; return m;
    }

    // Format a mention string for a user
    static std::string ping(const User &user) {
        return "<@" + user.id + ">";
    }

    // Send an image to a channel
    static Message send_picture(const Channel &channel, const std::string &image_url) {
        std::cout << "[send_picture] channel=" << channel.id << " image=" << image_url << std::endl;
        Message m; m.content = image_url; return m;
    }
};

} // namespace Wool
