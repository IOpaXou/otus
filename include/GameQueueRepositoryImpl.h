#pragma once

#include "IGameQueueRepository.h"

#include <string>
#include <unordered_map>

class GameQueueRepositoryImpl : public IGameQueueRepository
{
public:
    GameQueueRepositoryImpl(std::unordered_map<std::string, IQueuePtr<ICommandPtr>> queueMap) : _queueMap(std::move(queueMap)) {}
    IQueuePtr<ICommandPtr> findGameQueueById(const std::string& gameId)
    {
        const auto it = _queueMap.find(gameId);
        return it != _queueMap.end() ? it->second : IQueuePtr<ICommandPtr>();
    }

private:
    std::unordered_map<std::string, IQueuePtr<ICommandPtr>> _queueMap;
};
