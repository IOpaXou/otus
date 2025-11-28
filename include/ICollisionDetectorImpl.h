#include "ICollisionDetector.h"

#include <cmath>

class ICollisionDetectorImpl : public ICollisionDetector
{
public:
    ICollisionDetectorImpl(double colDistance = 2.0) : _colDistance(colDistance) {}
    bool checkCollision(IMovablePtr obj1, IMovablePtr obj2)
    {
        auto pos1 = obj1->getLocation();
        auto pos2 = obj2->getLocation();

        auto distance = std::sqrt(std::pow(pos1.first - pos2.first, 2.0) + std::pow(pos1.second - pos2.second, 2.0));
        
        return distance < _colDistance;
    }
private:
    double _colDistance;
};
