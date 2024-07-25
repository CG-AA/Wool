# Wool - The Discord Library

- very fast
- only do the basic stuff(handle heartbeat, send message that you gave it, etc)

## Features

- **Modern C++ Design**: Utilizes modern C++ standards to ensure a smooth development experience.
- **High Performance**: Optimized for performance to handle high-load bot interactions seamlessly.(but no sharding for now)
- **Easy to Use**: Usage of Wool is easy, handle message from Discord is another.

### todo
- **sharding**: Support for sharding to distribute bot load across multiple instances.(only will do if someone asks for it)
- **rate limiting**: Automatic rate limiting to prevent bot abuse and ensure compliance with Discord's API guidelines.
- **nonce support**: Support for nonces to prevent duplicate messages and ensure message integrity.

## Getting Started

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
    sudo apt-get install -y git cmake libcurl4-openssl-dev nlohmann-json3-dev libwebsocketpp-dev libspdlog-dev
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

4. Build the library:
    ```sh
    make
    ```

5. Install the library:
    ```sh
    make install
    ```

## Examples

You can find examples on how to use Wool in the `examples/` directory. These examples cover a range of use cases and demonstrate the library's capabilities.

## Contributing

Contributions are welcome! Please read the `CONTRIBUTING.md` file for more information on how to contribute to Wool.

## License

Wool is licensed under the MIT License. See the `LICENSE` file for more details.