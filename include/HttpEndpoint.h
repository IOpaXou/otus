#pragma once

#include "AuthService.h"
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
    void setCheckAuth(bool isCheckAuth);
    bool isCheckAuth();

private:
    void handleRequest(const httplib::Request& req, httplib::Response& res);
    void handleCreateGame(const httplib::Request& req, httplib::Response& res);
    void handleIssueToken(const httplib::Request& req, httplib::Response& res);

private:
    std::unique_ptr<httplib::Server> _server;
    std::string _host;
    int _port;
    bool _isRunning = false;
    bool _isCheckAuth = false;

    IGameQueueRepositoryPtr _gameQueueRepository;
    AuthServiceUPtr _authService;
};
