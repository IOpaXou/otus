#include "NeighborhoodSystem.h"

#include "CollisionDetectionCommand.h"

NeighborhoodSystem::NeighborhoodSystem(double cellSize, Vector offset, ICollisionDetectorPtr colDetector) 
    : _cellSize(cellSize), _offset(offset), _colDetector(colDetector)
{
}

NeighborhoodKey NeighborhoodSystem::getNeighborhoodKey(const Vector& pos) const
{
    return {
        static_cast<int>((pos.first + _offset.first) / _cellSize),
        static_cast<int>((pos.second + _offset.second) / _cellSize)
    };
}

void NeighborhoodSystem::addObject(IMovablePtr obj)
{
    auto key = getNeighborhoodKey(obj->getLocation());
    _neighborhoods[key].push_back(obj);
    _objectToNeighborhood[obj] = key;
    _neighborhoodToCmd[key] = ICommandPtr();
}

void NeighborhoodSystem::removeObject(IMovablePtr obj)
{
    auto it = _objectToNeighborhood.find(obj);
    if (it != _objectToNeighborhood.end())
    {
        auto& objects = _neighborhoods[it->second];
        objects.erase(std::remove(objects.begin(), objects.end(), obj), objects.end());
        _objectToNeighborhood.erase(it);

        _neighborhoodToCmd.erase(it->second);
    }
}

void NeighborhoodSystem::updateObject(IMovablePtr obj)
{
    auto cmd = std::make_unique<CollisionDetectionCommand>(obj, shared_from_this(), _colDetector);
    cmd->exec();
}

void NeighborhoodSystem::updateObjectPosition(IMovablePtr obj)
{
    auto oldKey = _objectToNeighborhood[obj];
    auto newKey = getNeighborhoodKey(obj->getLocation());
    if (oldKey != newKey)
    {
        removeObject(obj);
        addObject(obj);
    }
}

void NeighborhoodSystem::substituteCommand(NeighborhoodKey key, ICommandPtr cmd)
{
    _neighborhoodToCmd[key] = cmd;
}

void NeighborhoodSystem::checkCollisions()
{
    for (auto& [key, cmd] : _neighborhoodToCmd)
    {
        if (cmd)
        {
            cmd->exec();
        }
    }
}

std::vector<IMovablePtr> NeighborhoodSystem::getObjectsInNeighborhood(NeighborhoodKey key)
{
    auto it = _neighborhoods.find(key);
    if (it != _neighborhoods.end()) {
        return it->second;
    }
    return {};
}
