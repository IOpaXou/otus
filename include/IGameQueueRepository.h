#pragma once

#include "ICommand.h"
#include "IQueue.h"

class IGameQueueRepository
{
public:
    virtual IQueuePtr<ICommandPtr> createGameQueueWithId(const std::string& gameId) = 0;
    virtual IQueuePtr<ICommandPtr> findGameQueueById(const std::string& gameId) = 0;
};

using IGameQueueRepositoryPtr = std::shared_ptr<IGameQueueRepository>;
