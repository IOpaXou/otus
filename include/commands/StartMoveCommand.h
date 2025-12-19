#pragma once

#include "ICommand.h"
#include "UObject.h"

class StartMoveCommand : public ICommand
{
public:
    StartMoveCommand(UObjectPtr obj) : _obj(obj) {}
    void exec() override
    {
		auto initVelocity = _obj->getProperty<Vector>(UObject::InitialVelocityProperty);
		_obj->setProperty(UObject::VelocityProperty, initVelocity);
    }

private:
    UObjectPtr _obj;
};
