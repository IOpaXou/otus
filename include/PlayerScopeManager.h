#pragma once

#include "IObjectRepository.h"

#include <mutex>

class PlayerScopeManager
{
public:
    PlayerScopeManager(IObjectRepositoryPtr objRepository);
    void createPlayerScope(const std::string& playerId);
    void setCurrentPlayerScope(const std::string& playerId);
    void setDefaultScope();
    std::string getCurrentPlayer();
    void destroyPlayerScope(const std::string& playerId);

private:
    void registerPlayerDependencies(const std::string& playerId);
    UObjectPtr getObject(const std::string& playerId, const std::string& objectId);

private:
    mutable std::mutex _mutex;
    std::string _playerId;
    std::unordered_map<std::string, std::string> _playerScopes;
    IObjectRepositoryPtr _objRepository;
};

using PlayerScopeManagerPtr = std::shared_ptr<PlayerScopeManager>;
