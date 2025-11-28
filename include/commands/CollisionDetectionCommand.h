#pragma once

#include "CollisionCheckCommand.h"
#include "MacroCommand.h"
#include "NeighborhoodSystem.h"

class CollisionDetectionCommand : public ICommand
{
public:
    CollisionDetectionCommand(IMovablePtr obj, NeighborhoodSystemPtr nsystem, ICollisionDetectorPtr colDetector) 
        : _obj(obj), _nsystem(nsystem), _colDetector(colDetector)
    {}

    void exec() override
    {
        auto oldKey = _nsystem->getNeighborhoodKey(_obj->getLocation());
        _nsystem->updateObjectPosition(_obj);
        auto newKey = _nsystem->getNeighborhoodKey(_obj->getLocation());

        updateNeighborhoodMacro(oldKey);
        if (oldKey != newKey)
        {
            updateNeighborhoodMacro(newKey);
        }
    }

private:
    void updateNeighborhoodMacro(NeighborhoodKey key)
    {
        std::vector<IMovablePtr> nObjects = _nsystem->getObjectsInNeighborhood(key);

        std::vector<ICommandUPtr> collisionCommands;
        for (auto i = 0; i < nObjects.size(); i++)
            for (auto j = i + 1; j < nObjects.size(); j++)
            {
                auto colCmd = std::make_unique<CollisionCheckCommand>(nObjects[i], nObjects[j], _colDetector);
                collisionCommands.push_back(std::move(colCmd));
            }

        auto macroCmd = std::make_shared<MacroCommand>(std::move(collisionCommands));

        _nsystem->substituteCommand(key, macroCmd);
    }

private:
    NeighborhoodSystemPtr _nsystem;
    IMovablePtr _obj;
    ICollisionDetectorPtr _colDetector;
};
