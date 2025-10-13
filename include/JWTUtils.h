#pragma once

#include <string>

class JWTUtils
{
public:
    static void generateRSAKeys(std::string& private_key, std::string& public_key);
    static std::string generateToken(const std::string& user_id, 
                               const std::string& game_id,
                               const std::string& private_key);
    static bool verifyToken(const std::string& token, const std::string& public_key);

private:
    static std::string base64_encode(const std::string& in);
    static std::string base64_decode(const std::string& in);
    static std::string rsa_sign(const std::string& private_key, const std::string& data);
    static bool rsa_verify(const std::string& public_key,
                           const std::string& data,
                           const std::string& signature);
};
