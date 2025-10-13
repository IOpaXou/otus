#include "AuthService.h"
#include "JWTUtils.h"

#include <algorithm>
#include <sstream>

AuthService::AuthService()
{
    JWTUtils::generateRSAKeys(_secretKey, _publicKey);
}

std::string AuthService::createGame(const std::string& owner, const std::vector<std::string>& participants)
{
    std::lock_guard<std::mutex> lock(_mutex);
    
    GameInfo gameInfo;
    gameInfo.gameId = generateGameId();
    gameInfo.owner = owner;
    gameInfo.participants = participants;
    
    if (std::find(participants.begin(), participants.end(), owner) == participants.end())
    {
        gameInfo.participants.push_back(owner);
    }
    
    _games[gameInfo.gameId] = gameInfo;
    return gameInfo.gameId;
}

std::string AuthService::issueToken(const std::string& gameId, const std::string& userId)
{
    std::lock_guard<std::mutex> lock(_mutex);

    auto it = _games.find(gameId);
    if (it == _games.end()) {
        throw std::runtime_error("Game not found: " + gameId);
    }

    const GameInfo& game = it->second;
    if (std::find(game.participants.begin(), game.participants.end(), userId) == game.participants.end())
    {
        throw std::runtime_error("User '" + userId + "' is not a participant of game: " + gameId);
    }

    std::string token = JWTUtils::generateToken(userId, gameId, _secretKey);
    return token;
}

std::string AuthService::getPublicKey() const
{
    return _publicKey;
}

std::string AuthService::generateGameId()
{
    std::stringstream ss;
    ss << "game_" << ++_gamesCount << "_" << std::time(nullptr);
    return ss.str();
}
