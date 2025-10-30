#include "AuthRequest.h"
#include "AuthService.h"
#include "JWTUtils.h"
#include "GameInfo.h"
#include "json.hpp"

#include <algorithm>
#include <sstream>

AuthService::AuthService(const std::string& host, int port, IGameQueueRepositoryPtr gameQueueRepository) :
    _host(host), _port(port), _gameQueueRepository(gameQueueRepository)
{
    JWTUtils::generateRSAKeys(_secretKey, _publicKey);

    _server = std::make_unique<httplib::Server>();

    _server->Post("/auth/game/create", [this](const httplib::Request& req, httplib::Response& res)
    {
        this->handleCreateGame(req, res);
    });
    _server->Post("/auth/token", [this](const httplib::Request& req, httplib::Response& res)
    {
        this->handleIssueToken(req, res);
    });
    _server->Post("/auth/key/public", [this](const httplib::Request& req, httplib::Response& res)
    {
        this->handlePublicKey(req, res);
    });
}

AuthService::~AuthService()
{
    stop();
}

void AuthService::start()
{
    if (_isRunning)
    {
        return;
    }

    _isRunning = true;
    _serverThread = std::make_unique<std::thread>([this]() {
        _server->listen(_host, _port);
    });
}

void AuthService::stop()
{
    if (_server && _isRunning)
    {
        _server->stop();
        _isRunning = false;
    }

    if (_serverThread->joinable())
    {
        _serverThread->join();
    }
}

bool AuthService::isRunning() const
{
    return _isRunning;
}

void AuthService::handleCreateGame(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        GameInfo gameInfo = GameInfo::fromJSON(req.body);

        if (gameInfo.owner.empty()) {
            res.status = 400;
            res.set_content(R"({"error": "Owner is required"})", "application/json");
            return;
        }

        if (gameInfo.participants.empty()) {
            res.status = 400;
            res.set_content(R"({"error": "At least one participant is required"})", "application/json");
            return;
        }

        std::string gameId = createGame(gameInfo.owner, gameInfo.participants);

        auto gameQueue = _gameQueueRepository->createGameQueueWithId(gameId);

        res.status = 201;
        std::string response = R"({"status": "success", "gameId":")" + gameId + R"("})";
        res.set_content(response, "application/json");
    }
    catch (const std::exception& e)
    {
        res.status = 500;
        res.set_content(R"({"error": "Wrong game creation apply"})", "application/json");
    }
}

void AuthService::handleIssueToken(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        auto authReq = AuthRequest::fromJSON(req.body);

        if (authReq.gameId.empty() || authReq.userId.empty()) {
            res.status = 400;
            res.set_content(R"({"error": "userId and gameId are required"})", "application/json");
            return;
        }

        std::string token = issueToken(authReq.gameId, authReq.userId);

        res.status = 200;
        std::string response = "{\"token\": \"" + token + "\"}";
        res.set_content(response, "application/json");
    }
    catch (const std::exception& e)
    {
        res.status = 400;
        res.set_content(R"({"error": "Failed to issue token"})", "application/json");
    }
}

void AuthService::handlePublicKey(const httplib::Request& req, httplib::Response& res)
{
    res.status = 200;
    nlohmann::json response = {
        {"publicKey", _publicKey}
    };
    res.set_content(response.dump(), "application/json");
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

std::string AuthService::generateGameId()
{
    std::stringstream ss;
    ss << "game_" << ++_gamesCount << "_" << std::time(nullptr);
    return ss.str();
}
