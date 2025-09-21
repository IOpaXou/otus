#pragma once

#include "Defs.h"

class IFuelable
{
public:
    virtual FuelUnit getFuelLevel() = 0;
    virtual FuelUnit getFuelConsumptionValue() = 0;
    virtual void setFuelLevel(FuelUnit fUnit) = 0;
};
