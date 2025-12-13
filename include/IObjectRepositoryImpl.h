#pragma once

#include "IObjectRepository.h"
#include "UObject.h"

#include <unordered_map>

class IObjectRepositoryImpl : public IObjectRepository
{
public:
    IObjectRepositoryImpl() {}
    void addObject(const std::string& playerId, const std::string& objectId, UObjectPtr uObj) override
    {
        _objStorage[playerId][objectId] = uObj;
    }
    UObjectPtr getObject(const std::string& playerId, const std::string& objectId) override
    {
        auto it = _objStorage.find(playerId);
        if (_objStorage.find(playerId) != _objStorage.end())
        {
            const auto playerStorage = _objStorage.find(playerId)->second;
            if (playerStorage.find(objectId) != playerStorage.end())
            {
                return playerStorage.find(objectId)->second;
            }
        }
        return UObjectPtr();
    }

private:
    std::unordered_map<std::string, std::unordered_map<std::string, UObjectPtr>> _objStorage;
};
