#include "MultiNeighborhoodCollisionSystem.h"

MultiNeighborhoodCollisionSystem::MultiNeighborhoodCollisionSystem(double cellSize, ICollisionDetectorPtr collisionDetector)
    : _cellSize(cellSize), _collisionDetector(collisionDetector)
{
    _neighborhoodSystems.push_back(std::make_shared<NeighborhoodSystem>(_cellSize, Vector(0.0, 0.0), _collisionDetector));
    _neighborhoodSystems.push_back(std::make_shared<NeighborhoodSystem>(_cellSize, Vector(_cellSize / 2, _cellSize / 2), _collisionDetector));
}

MultiNeighborhoodCollisionSystem::MultiNeighborhoodCollisionSystem(double cellSize, int systemsCount, ICollisionDetectorPtr collisionDetector)
    : _cellSize(cellSize), _collisionDetector(collisionDetector)
{
    for (auto c = 0; c < systemsCount; c++)
    {
        _neighborhoodSystems.push_back(std::make_shared<NeighborhoodSystem>(_cellSize, 
            Vector(c * _cellSize / systemsCount, c * _cellSize / systemsCount), _collisionDetector));
    }
}

void MultiNeighborhoodCollisionSystem::addObject(IMovablePtr obj)
{
    for (auto system : _neighborhoodSystems) {
        system->addObject(obj);
    }
}

void MultiNeighborhoodCollisionSystem::removeObject(IMovablePtr obj)
{
    for (auto system : _neighborhoodSystems) {
        system->removeObject(obj);
    }
}

void MultiNeighborhoodCollisionSystem::updateObject(IMovablePtr obj)
{
    for (auto system : _neighborhoodSystems) {
        system->updateObject(obj);
    }
}

void MultiNeighborhoodCollisionSystem::checkAllCollisions()
{
    for (auto system : _neighborhoodSystems) {
        system->checkCollisions();
    }
}
