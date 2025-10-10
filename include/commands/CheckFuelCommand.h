#pragma once

#include "CommandException.h"

#include "ICommand.h"
#include "IFuelable.h"

namespace
{
    const auto NotEnoughFuelMessage = "Not enough fuel to start the engine";
}

class CheckFuelCommand : public ICommand
{
public:
    explicit CheckFuelCommand(IFuelablePtr fObj) : _fObj(fObj) {}
    void exec() override
    {
        if (_fObj->getFuelLevel() < _fObj->getFuelConsumptionValue())
        {
            throw CommandException(NotEnoughFuelMessage);
        }
    }

private:
    IFuelablePtr _fObj;
};
