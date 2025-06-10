# Wool

Simple C++ helpers for interacting with Discord's message REST API.

## Building

This library uses libcurl and nlohmann::json. On Ubuntu you can install them with:

```bash
sudo apt-get install libcurl4-openssl-dev nlohmann-json3-dev
```

Compile the example program using:

```bash
g++ -std=c++17 example.cpp Wool.cpp -o example -lcurl
```

`DISCORD_BOT_TOKEN` must be set to your bot token before running.

## Example

```
./example <channel_id> "Hello world"
```
