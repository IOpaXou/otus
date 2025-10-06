#include "HttpEndpoint.h"
#include "InterpretCommand.h"

HttpEndpoint::HttpEndpoint(const std::string& host, int port, IGameQueueRepositoryPtr gameQueueRepository) :
    _host(host), _port(port), _gameQueueRepository(gameQueueRepository)
{
    _server = std::make_unique<httplib::Server>();
    _server->Post("/command", [this](const httplib::Request& req, httplib::Response& res)
    {
        this->handleRequest(req, res);
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
