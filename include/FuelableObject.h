#pragma once

#include "IFuelable.h"

class FuelableObject : public IFuelable
{
public:
    FuelableObject(FuelUnit level, FuelUnit consumption) : fLevel(level), fConsumption(consumption) {}

    FuelUnit getFuelLevel() override
    {
        return fLevel;
    }

    FuelUnit getFuelConsumptionValue() override
    {
        return fConsumption;
    }

    void setFuelLevel(FuelUnit newFLevel) override
    {
        fLevel = newFLevel;
    }

private:
    FuelUnit fLevel;
    FuelUnit fConsumption;
};
