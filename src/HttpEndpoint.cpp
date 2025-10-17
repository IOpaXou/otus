#include "AuthRequest.h"
#include "AuthService.h"
#include "GameInfo.h"
#include "HttpEndpoint.h"
#include "InterpretCommand.h"
#include "JWTUtils.h"

HttpEndpoint::HttpEndpoint(const std::string& host, int port, IGameQueueRepositoryPtr gameQueueRepository) :
    _host(host), _port(port), _gameQueueRepository(gameQueueRepository), _authService(std::make_unique<AuthService>())
{
    _server = std::make_unique<httplib::Server>();
    _server->Post("/command", [this](const httplib::Request& req, httplib::Response& res)
    {
        this->handleRequest(req, res);
    });
    _server->Post("/auth/game/create", [this](const httplib::Request& req, httplib::Response& res)
    {
        this->handleCreateGame(req, res);
    });
    _server->Post("/auth/token", [this](const httplib::Request& req, httplib::Response& res)
    {
        this->handleIssueToken(req, res);
    });
}

HttpEndpoint::~HttpEndpoint()
{
    if (_isRunning)
    {
        stop();
    }
}

void HttpEndpoint::handleRequest(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        GameMessage message = GameMessage::fromJSON(req.body);
        if (isCheckAuth())
        {
            if (!JWTUtils::verifyToken(message.jwt, _authService->getPublicKey()))
            {
                res.status = 403;
                res.set_content(R"({"error": "Invalid token"})", "application/json");
                return;
            }
        }

        auto gameQueue =_gameQueueRepository->findGameQueueById(message.gameId);

        if (!gameQueue)
        {
            res.status = 404;
            res.set_content(R"({"error": "Game not found"})", "application/json");
            return;
        }

        auto interpretCommand = std::make_shared<InterpretCommand>(message, gameQueue);
        gameQueue->push(interpretCommand);

        res.status = 200;
        res.set_content(R"({"status": "accepted"})", "application/json");
    }
    catch (std::exception& ex)
    {
        res.status = 400;
        res.set_content(R"({"error": "Invalid message format"})", "application/json");
    }
}

void HttpEndpoint::handleCreateGame(const httplib::Request& req, httplib::Response& res)
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

        std::string gameId = _authService->createGame(gameInfo.owner, gameInfo.participants);

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

void HttpEndpoint::handleIssueToken(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        auto authReq = AuthRequest::fromJSON(req.body);

        if (authReq.gameId.empty() || authReq.userId.empty()) {
            res.status = 400;
            res.set_content(R"({"error": "userId and gameId are required"})", "application/json");
            return;
        }

        std::string token = _authService->issueToken(authReq.gameId, authReq.userId);

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

void HttpEndpoint::start()
{
    if (_isRunning)
    {
        return;
    }

    _isRunning = true;
    std::thread serverThread([this]() {
        _server->listen(_host, _port);
    });

    serverThread.join();
}

void HttpEndpoint::stop()
{
    if (_server && _isRunning)
    {
        _server->stop();
        _isRunning = false;
    }
}

bool HttpEndpoint::isRunning() const
{
    return _isRunning;
}

void HttpEndpoint::setCheckAuth(bool isCheckAuth)
{
    _isCheckAuth = isCheckAuth;
}

bool HttpEndpoint::isCheckAuth()
{
    return _isCheckAuth;
}
