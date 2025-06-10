# Wool

Simple C++ helpers for interacting with Discord's message REST API.

## Building

Run `scripts/setup.sh` to install dependencies and build the example and test binaries. This will create `example` and `tests/send_message_test`.

This library uses libcurl and nlohmann::json. On Ubuntu you can install them with:

```bash
sudo apt-get install libcurl4-openssl-dev nlohmann-json3-dev
```

You can also compile the example program manually using:

```bash
g++ -std=c++17 example.cpp Wool.cpp -o example -lcurl
```

`DISCORD_BOT_TOKEN` must be set to your bot token before running.

## Example

```
./example <channel_id> "Hello world"
```
