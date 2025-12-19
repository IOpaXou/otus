#pragma once

#include "ICommand.h"
#include "UObject.h"

class StopMoveCommand : public ICommand
{
public:
    StopMoveCommand(UObjectPtr obj) : _obj(obj) {}
    void exec() override
    {
		  _obj->setProperty(UObject::VelocityProperty, Vector(0, 0));
    }

private:
    UObjectPtr _obj;
};
