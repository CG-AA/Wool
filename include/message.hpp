#ifndef MESSAGE_HPP
#define MESSAGE_HPP
#define snowflake int64_t
class Message {
public:
    struct allowed_mentions{
        bool everyone = false;
        bool roles = true;
        bool users = true;
        std::vector<snowflake> roles_ids;
        std::vector<snowflake> users_ids;
        bool replied_user = false;
    };
    struct message_reference{
        int8_t type = 0;
        snowflake message_id;
        snowflake channel_id;
        bool fail_if_not_exists = true;
    };
    struct components{
        struct action_row{
            std::vector<std::any> components;
        };
        struct button{
            std::string style;
            std::string label;//optional
            std::string emoji;//optional
            std::string custom_id;//optional
            snowflake sku_id;//optional
            std::string url;//optional
            bool disabled = false;//optional
        };
        struct select_menu{
            int8_t type = 3;
            struct option{
                std::string label;
                std::string value;
                std::string description;//optional
                std::string emoji;//optional
                bool default = false;//optional
            };
            std::string custom_id;
            std::vector<option> options;//optional
            std::vector<std::string> channel_types;//optional
            std::string placeholder;//optional
            snowflake min_values;//optional
            snowflake max_values;//optional
            bool disabled = false;//optional
        };
        struct emoji{
            std::string name;
            snowflake id;
            bool animated = false;
        };
    };
    struct poll{
        
    };
    snowflake channelID;
    std::string content;
    // nonce (todo)
    bool tts = false;
    std::string embeds;
    nlohmann::json allowed_mentions;
    nlohmann::json message_reference;
    nlohmann::json components;
    std::vector<snowflake> sticker_ids;//I don't know what this is
    // files and payload_json (todo)
    // attachments (todo)
    snowflake flags = 0;
    // enforce_nonce (todo)
    nlohmann::json poll;
};

#endif