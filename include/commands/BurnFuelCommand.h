#pragma once

#include "ICommand.h"
#include "IFuelable.h"

class BurnFuelCommand : public ICommand
{
public:
    explicit BurnFuelCommand(IFuelablePtr fObj) : _fObj(fObj) {}
    void exec() override
    {
        const auto newFuelLevel = _fObj->getFuelLevel() - _fObj->getFuelConsumptionValue();
        _fObj->setFuelLevel(newFuelLevel);
    }

private:
    IFuelablePtr _fObj;
};
