#pragma once

#include "GameInfo.h"
#include "IGameQueueRepository.h"
#include "httplib.h"

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

class AuthService
{
public:
    AuthService(const std::string& host, int port, IGameQueueRepositoryPtr gameQueueRepository);
    ~AuthService();

    void start();
    void stop();
    bool isRunning() const;

private:
    void handleCreateGame(const httplib::Request& req, httplib::Response& res);
    void handleIssueToken(const httplib::Request& req, httplib::Response& res);
    void handlePublicKey(const httplib::Request& req, httplib::Response& res);

    std::string createGame(const std::string& owner, const std::vector<std::string>& participants);
    std::string issueToken(const std::string& gameId, const std::string& userId);

    std::string generateGameId();

private:
    std::unique_ptr<httplib::Server> _server;
    std::unique_ptr<std::thread> _serverThread;
    std::string _host;
    int _port;
    IGameQueueRepositoryPtr _gameQueueRepository;
    bool _isRunning = false;

    std::unordered_map<std::string, GameInfo> _games;
    int _gamesCount = 0;
    std::string _secretKey;
    std::string _publicKey;
    mutable std::mutex _mutex;
};

using AuthServiceUPtr = std::unique_ptr<AuthService>;
