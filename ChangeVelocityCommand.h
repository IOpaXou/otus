#pragma once

#include "ICommand.h"
#include "IMovable.h"
#include "IRotatable.h"

#include <cmath>

class ChangeVelocityCommand : public ICommand
{
public:
    explicit ChangeVelocityCommand(IRotatable* rObj, IMovable* mObj) : _rObj(rObj), _mObj(mObj) {}
    void exec() override
    {
        if (!_mObj)
        {
            return;
        }

        const auto velocity = _mObj->getVelocity();
        const auto x = velocity.first;
        const auto y = velocity.second;
        const auto angularVelocity = _rObj->getAngularVelocity();
        const auto angle = angularVelocity * M_PI / 180.0;

        const auto newX = x*std::cos(angle) - y*std::sin(angle);
        const auto newY = x*std::sin(angle) + y*std::cos(angle);

        _mObj->setVelocity(Vector(newX, newY));
    }

private:
    IRotatable* _rObj;
    IMovable* _mObj;
};
