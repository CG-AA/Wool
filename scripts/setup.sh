#!/usr/bin/env bash
set -e

# Install dependencies if missing
packages=(libcurl4-openssl-dev nlohmann-json3-dev)
missing=()
for pkg in "${packages[@]}"; do
    if ! dpkg -s "$pkg" >/dev/null 2>&1; then
        missing+=("$pkg")
    fi
done

if [ ${#missing[@]} -gt 0 ]; then
    echo "Installing missing packages: ${missing[*]}"
    sudo apt-get update
    sudo apt-get install -y "${missing[@]}"
fi

# Build example program
echo "Building example.cpp"
g++ -std=c++17 -I. example.cpp Wool.cpp -o example -lcurl

# Build test program
echo "Building tests/send_message_test.cpp"
g++ -std=c++17 -I. tests/send_message_test.cpp Wool.cpp -o tests/send_message_test -lcurl

cat <<'USAGE'

Build complete.

Usage:
  ./example <channel_id> "<message>"
  ./tests/send_message_test <channel_id>

Ensure the DISCORD_BOT_TOKEN environment variable is set before running these executables.
USAGE

