#pragma once

#include "IMovable.h"

class MovableObject : public IMovable
{
public:
    MovableObject(Point p, Vector vel) : pos(p), velocity(vel) {}

    Point getLocation() override
    {
        return pos;
    }

    Vector getVelocity() override
    {
        return velocity;
    }

    void setLocation(const Point& newPos) override
    {
        pos = newPos;
    }

private:
    Point pos;
    Vector velocity;
};
