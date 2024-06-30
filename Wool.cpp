#include "Wool.hpp"
#include <sodium.h>
#include <spdlog/spdlog.h>

void setupRoutes(uWS::App* Wool) {
    Wool->post("/DCendpoint", [&PUBKEY](auto* res, auto* req) {
        if (sodium_init() != 0) {
            spdlog::error("Failed to initialize libsodium");
            res->writeStatus("500 Internal Server Error")->end();
            throw std::runtime_error("Failed to initialize libsodium");
            return;
        }

        // Extract headers
        std::string signature = req->getHeader("x-signature-ed25519").toString();
        std::string timestamp = req->getHeader("x-signature-timestamp").toString();
        if (signature.empty() || timestamp.empty()) {
            spdlog::error("Missing required headers");
            res->writeStatus("400 Bad Request")->end();
            return;
        }

        // Read the request body
        std::string body;
        res->onData([&body](std::string_view data, bool last) { //weird
            body.append(data.data(), data.length());
            if (last) {
                // Convert the hex string public key and signature to binary
                unsigned char publicKey[crypto_sign_PUBLICKEYBYTES];
                unsigned char signatureBin[crypto_sign_BYTES];
                sodium_hex2bin(publicKey, sizeof(publicKey), PUBKEY.c_str(), PUBKEY.length(), NULL, NULL, NULL);
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
                res->writeStatus("200 OK")->end("\"type\":1")
            }
        });
    });
}
Wool::run() {
    uWS::App Wool;

    setupRoutes(&Wool);

    Wool.listen(port, [port](auto* listen_socket) {
        if (listen_socket) {
            spdlog::info("Listening on port {}", port);
        } else {
            spdlog::error("Failed to listen on port {}", port);
            throw std::runtime_error("Failed to listen on port " + std::to_string(port));
        }
    }).run();
}