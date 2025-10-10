#pragma once

#include <memory>

class IRotatable
{
public:
    virtual double getAngle() = 0;
    virtual double getAngularVelocity() = 0;
    virtual void setAngle(double) = 0;
};

using IRotatablePtr = std::shared_ptr<IRotatable>;
