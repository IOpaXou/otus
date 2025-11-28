#pragma once

#include "ICollisionDetector.h"
#include "ICommand.h"
#include "IMovable.h"

#include <iostream>

class CollisionCheckCommand : public ICommand
{
public:
    CollisionCheckCommand(IMovablePtr obj1, IMovablePtr obj2, ICollisionDetectorPtr colDetector) 
        : _obj1(obj1), _obj2(obj2), _colDetector(colDetector) {}

    void exec() override
    {
        if (_colDetector->checkCollision(_obj1, _obj2))
        {
            std::cout << "Collision" << std::endl;
        }
    }

private:
    IMovablePtr _obj1, _obj2;
    ICollisionDetectorPtr _colDetector;
};
