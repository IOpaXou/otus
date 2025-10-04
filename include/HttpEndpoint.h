#pragma once

#include "IGameQueueRepository.h"
#include "httplib.h"

#include <memory>
#include <string>

class HttpEndpoint
{
public:
    HttpEndpoint(const std::string& host, int port, IGameQueueRepositoryPtr gameQueueRepository);
    ~HttpEndpoint();

    void start();
    void stop();
    bool isRunning() const;

private:
    void handleRequest(const httplib::Request& req, httplib::Response& res);

private:
    std::unique_ptr<httplib::Server> _server;
    std::string _host;
    int _port;
    bool _isRunning = false;

    IGameQueueRepositoryPtr _gameQueueRepository;
};
