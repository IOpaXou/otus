#pragma once

class IRotatable
{
public:
    virtual double getAngle() = 0;
    virtual double getAngularVelocity() = 0;
    virtual void setAngle(double) = 0;
};
