#pragma once

#include "GameInfo.h"

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

class AuthService
{
public:
    AuthService();
    std::string createGame(const std::string& owner, const std::vector<std::string>& participants);
    std::string issueToken(const std::string& gameId, const std::string& userId);
    std::string getPublicKey() const;

private:
    std::string generateGameId();

private:
    std::unordered_map<std::string, GameInfo> _games;
    int _gamesCount = 0;
    std::string _secretKey;
    std::string _publicKey;
    mutable std::mutex _mutex;
};

using AuthServiceUPtr = std::unique_ptr<AuthService>;
