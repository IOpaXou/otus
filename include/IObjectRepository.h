#pragma once

#include "UObject.h"

class IObjectRepository
{
public:
    virtual void addObject(const std::string& playerId, const std::string& objectId, UObjectPtr uObj) = 0;
    virtual UObjectPtr getObject(const std::string& playerId, const std::string& objectId) = 0;
};

using IObjectRepositoryPtr = std::shared_ptr<IObjectRepository>;
