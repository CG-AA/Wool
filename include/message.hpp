#ifndef MESSAGE_HPP
#define MESSAGE_HPP

class Message {
public:
    struct allowed_mentions{
        bool everyone = false;
        bool roles = true;
        bool users = true;
        std::vector<int64_t> roles_ids;
        std::vector<int64_t> users_ids;
        bool replied_user = false;
    };
    struct message_reference{
        int8_t type = 0;
        int64_t message_id;
        int64_t channel_id;
        bool fail_if_not_exists = true;
    };
    struct components{
        std::vector<std::any> components;
    };
    struct poll{
        
    };
    int64_t channelID;
    std::string content;
    // nonce (todo)
    bool tts = false;
    std::string embeds;
    nlohmann::json allowed_mentions;
    nlohmann::json message_reference;
    nlohmann::json components;
    std::vector<int64_t> sticker_ids;//I don't know what this is
    // files and payload_json (todo)
    // attachments (todo)
    int64_t flags = 0;
    // enforce_nonce (todo)
};

#endif