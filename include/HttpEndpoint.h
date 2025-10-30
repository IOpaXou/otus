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
    void setAuthAddress(const std::string& host, int port);
    void setCheckAuth(bool isCheckAuth);
    bool isCheckAuth();

private:
    void handleRequest(const httplib::Request& req, httplib::Response& res);
    void handleCreateGame(const httplib::Request& req, httplib::Response& res);
    void handleIssueToken(const httplib::Request& req, httplib::Response& res);
    std::string requestPublicKey(const httplib::Request& req, httplib::Response& res);

    void forwardAuthRequest(const std::string& path, const httplib::Request& req, httplib::Response& res);

private:
    std::unique_ptr<httplib::Server> _server;
    std::unique_ptr<std::thread> _serverThread;
    std::string _host;
    int _port;
    std::string _authHost;
    int _authPort;
    bool _isRunning = false;
    bool _isCheckAuth = false;

    IGameQueueRepositoryPtr _gameQueueRepository;
    std::unique_ptr<httplib::Client> _authClient;
};
