#pragma once

#include "IRotatable.h"

#include <cmath>

class RotateCommand
{
public:
    RotateCommand(IRotatable& rObj) : _rObj(rObj) {}
    
    void exec()
    {
        const auto angle = _rObj.getAngle();
        const auto angVelocity = _rObj.getAngularVelocity();

        auto newAngle = std::fmod(angle + angVelocity, 360.0);
        if (newAngle < 0)
        {
            newAngle += 360.0;
        }
        _rObj.setAngle(newAngle);
    }

private:
    IRotatable& _rObj;
};
