#pragma once

#include "IRotatable.h"

class RotatableObject : public IRotatable
{
public:
    RotatableObject(double angle, double angularVelocity) : _angle(angle), _angularVelocity(angularVelocity) {}

    double getAngle() override
    {
        return _angle;
    }

    double getAngularVelocity() override
    {
        return _angularVelocity;
    }

    void setAngle(double angle) override
    {
        _angle = angle;
    }

private:
    double _angle;
    double _angularVelocity;
};

using RotatableObjectPtr = std::shared_ptr<RotatableObject>;
