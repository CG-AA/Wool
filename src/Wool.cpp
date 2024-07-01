#include "../include/Wool.hpp"
#include <sodium.h>
#include <spdlog/spdlog.h>

void Wool::setupRoutes() {
    if(this->PUBKEY.empty()){
        spdlog::error("Missing PUBKEY");
        throw std::runtime_error("Missing PUBKEY");
    }
    this->app.post("/DCendpoint", [this](auto* res, auto* req) {
        // Extract headers
        std::string signature = std::string(req->getHeader("x-signature-ed25519"));
        std::string timestamp = std::string(req->getHeader("x-signature-timestamp"));
        if (signature.empty() || timestamp.empty()) {
            spdlog::error("Missing required headers");
            res->writeStatus("400 Bad Request")->end();
            return;
        }

        // Read the request body
        std::string body;
        res->onData([&body, &signature, &timestamp, res, this](std::string_view data, bool last) {
            body.append(data.data(), data.length());
            if (last) {
                // Convert the hex string public key and signature to binary
                unsigned char publicKey[crypto_sign_PUBLICKEYBYTES];
                unsigned char signatureBin[crypto_sign_BYTES];
                sodium_hex2bin(publicKey, sizeof(publicKey), this->PUBKEY.c_str(), this->PUBKEY.length(), NULL, NULL, NULL);
                sodium_hex2bin(signatureBin, sizeof(signatureBin), signature.c_str(), signature.length(), NULL, NULL, NULL);

                // Concatenate timestamp and body
                std::string message = timestamp + body;

                // Verify the signature
                if (crypto_sign_verify_detached(signatureBin, reinterpret_cast<const unsigned char*>(message.c_str()), message.length(), publicKey) != 0) {
                    // Verification failed
                    spdlog::error("Invalid signature");
                    res->writeStatus("401 Unauthorized")->end();
                    return;
                }

                // If verification is successful, process the request
                spdlog::info("Valid signature, processing request");
                res->writeHeader("Content-Type", "application/json")->writeStatus("200 OK")->end("\"type\":1");
            }
        });
    });
}

Wool::Wool() {
    if (sodium_init() == -1) {
        spdlog::error("Failed to initialize libsodium");
        throw std::runtime_error("Failed to initialize libsodium");
    }
}

void Wool::run() {
    this->setupRoutes();

    this->app.listen(this->port, [this](auto* listen_socket) {
        if (listen_socket) {
            spdlog::info("Listening on port {}", this->port);
        } else {
            spdlog::error("Failed to listen on port {}", this->port);
            throw std::runtime_error("Failed to listen on port " + std::to_string(this->port));
        }
    }).run();
}