# Goal
Send a message to a channel, ping a user, and send a picture.

```
#include "Wool.hpp"

int main() {
    Wool::Channel channel{"123"};
    Wool::User user{"456"};
    Wool::Wool::send_message(channel, "Hello " + Wool::Wool::ping(user));
    Wool::Wool::send_picture(channel, "https://example.com/cat.png");
}
```
