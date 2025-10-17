#pragma once

#include "IGameQueueRepository.h"
#include "IQueueImpl.h"

#include <string>
#include <unordered_map>

class GameQueueRepositoryImpl : public IGameQueueRepository
{
public:
    GameQueueRepositoryImpl(std::unordered_map<std::string, IQueuePtr<ICommandPtr>> queueMap) : _queueMap(std::move(queueMap)) {}

    IQueuePtr<ICommandPtr> createGameQueueWithId(const std::string& gameId) override
    {
        const auto it = _queueMap.find(gameId);
        if (it != _queueMap.end())
        {
            return it->second;
        }

        _queueMap[gameId] = std::make_shared<IQueueImpl<ICommandPtr>>();
        return _queueMap[gameId];
    }

    IQueuePtr<ICommandPtr> findGameQueueById(const std::string& gameId) override
    {
        const auto it = _queueMap.find(gameId);
        return it != _queueMap.end() ? it->second : IQueuePtr<ICommandPtr>();
    }

private:
    std::unordered_map<std::string, IQueuePtr<ICommandPtr>> _queueMap;
};
