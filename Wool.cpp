// Implementation of Discord message REST API using libcurl
// Helper functions convert between JSON and Message objects
// Only a small subset of fields is currently supported

#include "Wool.hpp"
#include "Logger.hpp"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <cstdlib>

namespace Wool {

void to_json(nlohmann::json& j, const Snowflake& s) {
    j = s.value;
}

void from_json(const nlohmann::json& j, Snowflake& s) {
    s.value = j.get<unsigned long long>();
}

namespace {

// Global logger instance used for all library logging
static Logger logger;

constexpr const char* API_BASE = "https://discord.com/api/v10";

// Curl callback to accumulate HTTP response data

size_t write_cb(void* contents, size_t size, size_t nmemb, void* userp) {
    std::string* out = static_cast<std::string*>(userp);
    out->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}
// Read the bot token from environment variables for Authorization header

std::string auth_header() {
    const char* token = std::getenv("DISCORD_BOT_TOKEN");
    if (!token) throw std::runtime_error("DISCORD_BOT_TOKEN not set");
    return std::string("Authorization: Bot ") + token;
}
// Structure capturing an HTTP response body and status
struct HttpResponse {
    std::string body;
    long status = 0;
};

// Perform an HTTP request using libcurl and return the body and status code
HttpResponse http_request(const std::string& method, const std::string& url,
                          const std::string& body = "") {
    logger.log(Logger::Level::Info, "HTTP " + method + " " + url);
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("curl_easy_init failed");
    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
    if (method == "POST" || method == "PATCH" || method == "PUT") {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    }
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, auth_header().c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    CURLcode res = curl_easy_perform(curl);
    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    logger.log(Logger::Level::Debug,
               "HTTP response received: " + std::to_string(res) +
                   " status=" + std::to_string(status));
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK) throw std::runtime_error("curl_easy_perform failed");
    return {response, status};
}
// Deserialize a subset of the Discord Message object from JSON


Message from_json(const nlohmann::json& j) {
    Message m;
    try {
        m.id = j.at("id").get<std::string>();
        m.channel_id = j.at("channel_id").get<std::string>();
        if (j.contains("author")) m.author.id = j["author"].at("id").get<std::string>();
        if (j.contains("content")) m.content = j["content"].get<std::string>();
        if (j.contains("timestamp")) m.timestamp = j["timestamp"].get<std::string>();
        if (j.contains("edited_timestamp") && !j["edited_timestamp"].is_null())
            m.edited_timestamp = j["edited_timestamp"].get<std::string>();
        if (j.contains("tts")) m.tts = j["tts"].get<bool>();
        if (j.contains("mention_everyone")) m.mention_everyone = j["mention_everyone"].get<bool>();
        if (j.contains("flags")) m.flags = j["flags"].get<int>();
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Invalid message JSON: ") + e.what());
    }

    return m;
}
// Serialize minimal Message fields to JSON for sending

std::string to_json(const Message& m) {
    nlohmann::json j;
    j["content"] = m.content;
    return j.dump();
}

} // namespace

// Retrieve recent messages from a channel

std::vector<Message> Message::GetChannelMessages(Snowflake channel_id) {
    logger.log(Logger::Level::Info, "GetChannelMessages channel_id=" + std::to_string(channel_id));
    std::string url = std::string(API_BASE) + "/channels/" + std::to_string(channel_id) + "/messages";
    auto resp = http_request("GET", url);
    try {
        auto arr = nlohmann::json::parse(resp.body);
        std::vector<Message> msgs;
        for (auto& itm : arr) msgs.push_back(from_json(itm));
        return msgs;
    } catch (const std::exception& e) {
        logger.log(Logger::Level::Error, std::string("GetChannelMessages parse error: ") + e.what());
        throw;
    }
}

// Fetch a single message by ID

Message Message::GetChannelMessage(Snowflake channel_id, Snowflake message_id) {
    logger.log(Logger::Level::Info, "GetChannelMessage channel_id=" + std::to_string(channel_id) + " message_id=" + std::to_string(message_id));
    std::string url = std::string(API_BASE) + "/channels/" + std::to_string(channel_id) + "/messages/" + std::to_string(message_id);
    auto resp = http_request("GET", url);
    try {
        return from_json(nlohmann::json::parse(resp.body));
    } catch (const std::exception& e) {
        logger.log(Logger::Level::Error, std::string("GetChannelMessage parse error: ") + e.what());
        throw;
    }
}

// Send a new message to a channel

Message Message::CreateMessage(Snowflake channel_id, const Message& msg) {
    logger.log(Logger::Level::Info, "CreateMessage channel_id=" + std::to_string(channel_id));
    std::string url = std::string(API_BASE) + "/channels/" + std::to_string(channel_id) + "/messages";
    auto resp = http_request("POST", url, to_json(msg));
    try {
        logger.log(Logger::Level::Debug, "CreateMessage response: " + resp.body);
        return from_json(nlohmann::json::parse(resp.body));
    } catch (const std::exception& e) {
        logger.log(Logger::Level::Error, std::string("CreateMessage parse error: ") + e.what());
        throw;
    }
}

// Crosspost a message to followers

Message Message::CrosspostMessage(Snowflake channel_id, Snowflake message_id) {
    logger.log(Logger::Level::Info, "CrosspostMessage channel_id=" + std::to_string(channel_id) + " message_id=" + std::to_string(message_id));
    std::string url = std::string(API_BASE) + "/channels/" + std::to_string(channel_id) + "/messages/" + std::to_string(message_id) + "/crosspost";
    auto resp = http_request("POST", url);
    try {
        return from_json(nlohmann::json::parse(resp.body));
    } catch (const std::exception& e) {
        logger.log(Logger::Level::Error, std::string("CrosspostMessage parse error: ") + e.what());
        throw;
    }
}

// Add a reaction using the current user

void Message::CreateReaction(const std::string& emoji) {
    logger.log(Logger::Level::Debug, "CreateReaction message_id=" + std::to_string(id) + " emoji=" + emoji);
    std::string url = std::string(API_BASE) + "/channels/" + std::to_string(channel_id) + "/messages/" + std::to_string(id) + "/reactions/" + curl_easy_escape(nullptr, emoji.c_str(), 0) + "/@me";
    http_request("PUT", url);
}

// Remove our own reaction

void Message::DeleteOwnReaction(const std::string& emoji) {
    logger.log(Logger::Level::Debug, "DeleteOwnReaction message_id=" + std::to_string(id) + " emoji=" + emoji);
    std::string url = std::string(API_BASE) + "/channels/" + std::to_string(channel_id) + "/messages/" + std::to_string(id) + "/reactions/" + curl_easy_escape(nullptr, emoji.c_str(), 0) + "/@me";
    http_request("DELETE", url);
}

// Remove another user's reaction

void Message::DeleteUserReaction(const std::string& emoji, Snowflake user_id) {
    logger.log(Logger::Level::Debug, "DeleteUserReaction message_id=" + std::to_string(id) + " user_id=" + std::to_string(user_id) + " emoji=" + emoji);
    std::string url = std::string(API_BASE) + "/channels/" + std::to_string(channel_id) + "/messages/" + std::to_string(id) + "/reactions/" + curl_easy_escape(nullptr, emoji.c_str(), 0) + "/" + std::to_string(user_id);
    http_request("DELETE", url);
}

// List users that reacted with a specific emoji

std::vector<User> Message::GetReactions(const std::string& emoji) {
    logger.log(Logger::Level::Debug, "GetReactions message_id=" + std::to_string(id) + " emoji=" + emoji);
    std::string url = std::string(API_BASE) + "/channels/" + std::to_string(channel_id) + "/messages/" + std::to_string(id) + "/reactions/" + curl_easy_escape(nullptr, emoji.c_str(), 0);
    auto resp = http_request("GET", url);
    try {
        auto arr = nlohmann::json::parse(resp.body);
        std::vector<User> users;
        for (auto& itm : arr) {
            User u{itm.at("id").get<std::string>()};
            users.push_back(u);
        }
        return users;
    } catch (const std::exception& e) {
        logger.log(Logger::Level::Error, std::string("GetReactions parse error: ") + e.what());
        throw;
    }
}

// Remove all reactions from this message

void Message::DeleteAllReactions() {
    logger.log(Logger::Level::Info, "DeleteAllReactions message_id=" + std::to_string(id));
    std::string url = std::string(API_BASE) + "/channels/" + std::to_string(channel_id) + "/messages/" + std::to_string(id) + "/reactions";
    http_request("DELETE", url);
}

// Remove all reactions for a single emoji

void Message::DeleteAllReactionsForEmoji(const std::string& emoji) {
    logger.log(Logger::Level::Info, "DeleteAllReactionsForEmoji message_id=" + std::to_string(id) + " emoji=" + emoji);
    std::string url = std::string(API_BASE) + "/channels/" + std::to_string(channel_id) + "/messages/" + std::to_string(id) + "/reactions/" + curl_easy_escape(nullptr, emoji.c_str(), 0);
    http_request("DELETE", url);
}

// Update this message

void Message::EditMessage(const Message& msg) {
    logger.log(Logger::Level::Info, "EditMessage message_id=" + std::to_string(id));
    std::string url = std::string(API_BASE) + "/channels/" + std::to_string(channel_id) + "/messages/" + std::to_string(id);
    http_request("PATCH", url, to_json(msg));
}

// Delete this message

void Message::DeleteMessage() {
    logger.log(Logger::Level::Warn, "DeleteMessage message_id=" + std::to_string(id));
    std::string url = std::string(API_BASE) + "/channels/" + std::to_string(channel_id) + "/messages/" + std::to_string(id);
    http_request("DELETE", url);
}

// Delete multiple messages at once

void Message::BulkDeleteMessages(Snowflake channel_id, const std::vector<Snowflake>& ids) {
    logger.log(Logger::Level::Warn, "BulkDeleteMessages channel_id=" + std::to_string(channel_id));
    std::string url = std::string(API_BASE) + "/channels/" + std::to_string(channel_id) + "/messages/bulk-delete";
    nlohmann::json j;
    j["messages"] = ids;
    http_request("POST", url, j.dump());
}

} // namespace Wool
