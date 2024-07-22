# Wool - The Discord Library

Wool is a modern C++ library designed to make Discord bot development easier and more intuitive. By leveraging the power of C++, Wool provides developers with a robust and efficient way to create complex and highly responsive bots for Discord.

## Features

- **Modern C++ Design**: Utilizes modern C++ standards to ensure a smooth development experience.
- **High Performance**: Optimized for performance to handle high-load bot interactions seamlessly.
- **Easy to Use**: Simplified API design for ease of use without sacrificing flexibility.

### todo
- **sharding**: Support for sharding to distribute bot load across multiple instances.
- **rate limiting**: Automatic rate limiting to prevent bot abuse and ensure compliance with Discord's API guidelines.
- **nonce support**: Support for nonces to prevent duplicate messages and ensure message integrity.

## Getting Started

### Prerequisites

- CMake 3.15 or higher
- A C++ compiler that supports at least C++20
- curl
- nlohmann/json
- websocketpp

### Building from Source using CMake

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