#pragma once

#include "ICollisionDetector.h"
#include "ICommand.h"
#include "IMovable.h"

#include <unordered_map>

struct NeighborhoodKey
{
    int x, y;

    bool operator==(const NeighborhoodKey& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const NeighborhoodKey& other) const {
        return !(operator==(other));
    }
};

namespace std {
    template<>
    struct hash<NeighborhoodKey> {
        size_t operator()(const NeighborhoodKey& key) const {
            return hash<int>()(key.x) ^ (hash<int>()(key.y) << 1);
        }
    };
}

class NeighborhoodSystem : public std::enable_shared_from_this<NeighborhoodSystem>
{
public:
    NeighborhoodSystem(double cellSize, Vector offset, ICollisionDetectorPtr colDetector);
    NeighborhoodKey getNeighborhoodKey(const Vector& position) const;
    void addObject(IMovablePtr obj);
    void removeObject(IMovablePtr obj);
    void updateObject(IMovablePtr obj);
    void updateObjectPosition(IMovablePtr mObj);
    virtual void substituteCommand(NeighborhoodKey key, ICommandPtr cmd);
    void checkCollisions();
    std::vector<IMovablePtr> getObjectsInNeighborhood(NeighborhoodKey key);

private:
    double _cellSize;
    Vector _offset;
    ICollisionDetectorPtr _colDetector;
    std::unordered_map<NeighborhoodKey, std::vector<IMovablePtr>> _neighborhoods;
    std::unordered_map<IMovablePtr, NeighborhoodKey> _objectToNeighborhood;
    std::unordered_map<NeighborhoodKey, ICommandPtr> _neighborhoodToCmd;
};

using NeighborhoodSystemPtr = std::shared_ptr<NeighborhoodSystem>;
