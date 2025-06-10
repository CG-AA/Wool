// Implementation of Discord message REST API using libcurl
// Helper functions convert between JSON and Message objects
// Only a small subset of fields is currently supported

#include "Wool.hpp"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <cstdlib>

namespace Wool {
namespace {

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
// Perform an HTTP request using libcurl and return the response body
// TODO: expose status codes and error handling

std::string http_request(const std::string& method, const std::string& url, const std::string& body = "") {
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
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK) throw std::runtime_error("curl_easy_perform failed");
    return response;
}
// Deserialize a subset of the Discord Message object from JSON


Message from_json(const nlohmann::json& j) {
    Message m;
    m.id = j.at("id").get<Snowflake>();
    m.channel_id = j.at("channel_id").get<Snowflake>();
    if (j.contains("author")) m.author.id = j["author"].at("id").get<Snowflake>();
    if (j.contains("content")) m.content = j["content"].get<std::string>();
    if (j.contains("timestamp")) m.timestamp = j["timestamp"].get<std::string>();
    if (j.contains("edited_timestamp") && !j["edited_timestamp"].is_null())
        m.edited_timestamp = j["edited_timestamp"].get<std::string>();
    if (j.contains("tts")) m.tts = j["tts"].get<bool>();
    if (j.contains("mention_everyone")) m.mention_everyone = j["mention_everyone"].get<bool>();
    if (j.contains("flags")) m.flags = j["flags"].get<int>();

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
    std::string url = std::string(API_BASE) + "/channels/" + std::to_string(channel_id) + "/messages";
    auto resp = http_request("GET", url);
    auto arr = nlohmann::json::parse(resp);
    std::vector<Message> msgs;
    for (auto& itm : arr) msgs.push_back(from_json(itm));
    return msgs;
}

// Fetch a single message by ID

Message Message::GetChannelMessage(Snowflake channel_id, Snowflake message_id) {
    std::string url = std::string(API_BASE) + "/channels/" + std::to_string(channel_id) + "/messages/" + std::to_string(message_id);
    auto resp = http_request("GET", url);
    return from_json(nlohmann::json::parse(resp));
}

// Send a new message to a channel

Message Message::CreateMessage(Snowflake channel_id, const Message& msg) {
    std::string url = std::string(API_BASE) + "/channels/" + std::to_string(channel_id) + "/messages";
    auto resp = http_request("POST", url, to_json(msg));
    return from_json(nlohmann::json::parse(resp));
}

// Crosspost a message to followers

Message Message::CrosspostMessage(Snowflake channel_id, Snowflake message_id) {
    std::string url = std::string(API_BASE) + "/channels/" + std::to_string(channel_id) + "/messages/" + std::to_string(message_id) + "/crosspost";
    auto resp = http_request("POST", url);
    return from_json(nlohmann::json::parse(resp));
}

// Add a reaction using the current user

void Message::CreateReaction(const std::string& emoji) {
    std::string url = std::string(API_BASE) + "/channels/" + std::to_string(channel_id) + "/messages/" + std::to_string(id) + "/reactions/" + curl_easy_escape(nullptr, emoji.c_str(), 0) + "/@me";
    http_request("PUT", url);
}

// Remove our own reaction

void Message::DeleteOwnReaction(const std::string& emoji) {
    std::string url = std::string(API_BASE) + "/channels/" + std::to_string(channel_id) + "/messages/" + std::to_string(id) + "/reactions/" + curl_easy_escape(nullptr, emoji.c_str(), 0) + "/@me";
    http_request("DELETE", url);
}

// Remove another user's reaction

void Message::DeleteUserReaction(const std::string& emoji, Snowflake user_id) {
    std::string url = std::string(API_BASE) + "/channels/" + std::to_string(channel_id) + "/messages/" + std::to_string(id) + "/reactions/" + curl_easy_escape(nullptr, emoji.c_str(), 0) + "/" + std::to_string(user_id);
    http_request("DELETE", url);
}

// List users that reacted with a specific emoji

std::vector<User> Message::GetReactions(const std::string& emoji) {
    std::string url = std::string(API_BASE) + "/channels/" + std::to_string(channel_id) + "/messages/" + std::to_string(id) + "/reactions/" + curl_easy_escape(nullptr, emoji.c_str(), 0);
    auto resp = http_request("GET", url);
    auto arr = nlohmann::json::parse(resp);
    std::vector<User> users;
    for (auto& itm : arr) {
        User u{itm.at("id").get<Snowflake>()};
        users.push_back(u);
    }
    return users;
}

// Remove all reactions from this message

void Message::DeleteAllReactions() {
    std::string url = std::string(API_BASE) + "/channels/" + std::to_string(channel_id) + "/messages/" + std::to_string(id) + "/reactions";
    http_request("DELETE", url);
}

// Remove all reactions for a single emoji

void Message::DeleteAllReactionsForEmoji(const std::string& emoji) {
    std::string url = std::string(API_BASE) + "/channels/" + std::to_string(channel_id) + "/messages/" + std::to_string(id) + "/reactions/" + curl_easy_escape(nullptr, emoji.c_str(), 0);
    http_request("DELETE", url);
}

// Update this message

void Message::EditMessage(const Message& msg) {
    std::string url = std::string(API_BASE) + "/channels/" + std::to_string(channel_id) + "/messages/" + std::to_string(id);
    http_request("PATCH", url, to_json(msg));
}

// Delete this message

void Message::DeleteMessage() {
    std::string url = std::string(API_BASE) + "/channels/" + std::to_string(channel_id) + "/messages/" + std::to_string(id);
    http_request("DELETE", url);
}

// Delete multiple messages at once

void Message::BulkDeleteMessages(Snowflake channel_id, const std::vector<Snowflake>& ids) {
    std::string url = std::string(API_BASE) + "/channels/" + std::to_string(channel_id) + "/messages/bulk-delete";
    nlohmann::json j;
    j["messages"] = ids;
    http_request("POST", url, j.dump());
}

} // namespace Wool
