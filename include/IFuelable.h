#pragma once

#include "Defs.h"

#include <memory>

class IFuelable
{
public:
    virtual FuelUnit getFuelLevel() = 0;
    virtual FuelUnit getFuelConsumptionValue() = 0;
    virtual void setFuelLevel(FuelUnit fUnit) = 0;
};

using IFuelablePtr = std::shared_ptr<IFuelable>;
