#pragma once

#include "Defs.h"

class IMovable
{
public:
	virtual Point getLocation() = 0;
	virtual Vector getVelocity() = 0;
	virtual void setLocation(const Point&) = 0;
};
