#pragma once

#include "NeighborhoodSystem.h"

class MultiNeighborhoodCollisionSystem
{
public:
    MultiNeighborhoodCollisionSystem(double cellSize, ICollisionDetectorPtr collisionDetector);
    MultiNeighborhoodCollisionSystem(double cellSize, int systemsCount, ICollisionDetectorPtr collisionDetector);
    void addObject(IMovablePtr obj);
    void removeObject(IMovablePtr obj);
    void updateObject(IMovablePtr obj);
    void checkAllCollisions();

private:
    double _cellSize;
    ICollisionDetectorPtr _collisionDetector;
    std::vector<NeighborhoodSystemPtr> _neighborhoodSystems;
    std::vector<ICommandPtr> _collisionCommands;
};
