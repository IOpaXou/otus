#include "HttpEndpoint.h"
#include "InterpretCommand.h"
#include "JWTUtils.h"
#include "httplib.h"
#include "json.hpp"

HttpEndpoint::HttpEndpoint(const std::string& host, int port, IGameQueueRepositoryPtr gameQueueRepository) :
    _host(host), _port(port), _gameQueueRepository(gameQueueRepository)
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
    stop();
}

void HttpEndpoint::handleRequest(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        GameMessage message = GameMessage::fromJSON(req.body);
        if (isCheckAuth())
        {
            const auto publicKey = requestPublicKey(req, res);
            if (publicKey.empty())
            {
                res.status = 403;
                res.set_content(R"({"error": "Can't get public key"})", "application/json");
                return;
            }

            if (!JWTUtils::verifyToken(message.jwt, publicKey))
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
    forwardAuthRequest("/auth/game/create", req, res);
}

void HttpEndpoint::handleIssueToken(const httplib::Request& req, httplib::Response& res)
{
    forwardAuthRequest("/auth/token", req, res);
}

std::string HttpEndpoint::requestPublicKey(const httplib::Request& req, httplib::Response& res)
{
    auto authRes = _authClient->Post("/auth/key/public", req.body, "application/json");
    if (!authRes || authRes->status != 200)
    {
        return "";
    }

    auto json = nlohmann::json::parse(authRes->body);
    return json["publicKey"];
}

void HttpEndpoint::forwardAuthRequest(const std::string& path, const httplib::Request& req, httplib::Response& res)
{
    if (!_authClient)
    {
        res.status = 502;
        res.set_content(R"({"error": "Auth service unavailable"})", "application/json");
        return;
    }

   try
   {
        auto authRes = _authClient->Post(path, req.body, "application/json");

        if (authRes) {
            res.status = authRes->status;
            res.set_content(authRes->body, authRes->get_header_value("Content-Type").c_str());
        } else {
            res.status = 502;
            res.set_content(R"({"error": "Auth service unavailable"})", "application/json");
        }
    }
    catch (const std::exception& e) {
        res.status = 500;
        res.set_content(R"({"error": "Internal server error"})", "application/json");
    }
}

void HttpEndpoint::start()
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

void HttpEndpoint::stop()
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

bool HttpEndpoint::isRunning() const
{
    return _isRunning;
}

void HttpEndpoint::setAuthAddress(const std::string& host, int port)
{
    _authHost = host;
    _authPort = port;
}

void HttpEndpoint::setCheckAuth(bool isCheckAuth)
{
    if (_isCheckAuth != isCheckAuth)
    {
        _isCheckAuth = isCheckAuth;

        if (_isCheckAuth && !_authClient)
        {
            _authClient = std::make_unique<httplib::Client>(_authHost, _authPort);
        }
    }
}

bool HttpEndpoint::isCheckAuth()
{
    return _isCheckAuth;
}
