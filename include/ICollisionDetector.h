#pragma once

#include "IMovable.h"

class ICollisionDetector
{
public:
    virtual bool checkCollision(IMovablePtr obj1, IMovablePtr obj2) = 0;
};

using ICollisionDetectorPtr = std::shared_ptr<ICollisionDetector>;
