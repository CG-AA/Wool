# Wool - The Discord Library

- very fast
- only do the basic stuff(handle heartbeat, send message that you gave it, etc)

## What does Wool do?
- gateway connection(for real-time communication, including someone send a message or someone just joined a voice channel)
- HTTP request(Rest API, for example: send message on a channel, get user info, etc)

## Features

- **Modern C++ Design**: Utilizes modern C++ standards to ensure a smooth development experience.
- **High Performance**: Optimized for performance to handle high-load bot interactions seamlessly.(but no sharding for now)
- **Easy to Use**: Usage of Wool is easy, handle message from Discord is another.

### todo
- **sharding**: Support for sharding to distribute bot load across multiple instances.(only will do if someone asks for it)
- **rate limiting**: Automatic rate limiting to prevent bot abuse and ensure compliance with Discord's API guidelines.
- **nonce support**: Support for nonces to prevent duplicate messages and ensure message integrity.

## Getting Started

### FAQ
if you have any question(literally anything), just open a issue.

### Prerequisites

- CMake 3.15 or higher
- A C++ compiler that supports at least C++17
- cURL
- nlohmann/json
- websocketpp
- spdlog

### Building from Source using CMake (Linux)

0. Install the required dependencies:
    ```sh
    sudo apt-get update
    sudo apt-get install -y git g++ cmake libcurl4-openssl-dev nlohmann-json3-dev libwebsocketpp-dev libspdlog-dev libboost-all-dev libssl-dev
    ```

1. Clone the repository:
    ```sh
    git clone https://github.com/CG-AA/Wool.git
    cd Wool
    ```

2. Create a build directory:
    ```sh
    mkdir build && cd build
    ```

3. Configure the project with CMake:
   ```sh
   cmake ..
   ```
    You can also specify the build type by adding `-DB_LIB=(ON/OFF)` or `-DB_EX=(ON/OFF)` to the `cmake` command.
    (you needs to build the library first, then you can build the examples)

4. Build the library:
    ```sh
    make
    ```

5. Install the library:
    ```sh
    sudo make install
    ```

## Examples

You can find example on how to use Wool in the `examples/` directory. and yes, it only have one example for now.

## Contributing

Please do it, I'm not good at C++.
just open a issue or a pull request.

## License

Wool is licensed under the MIT License. See the `LICENSE` file for more details.