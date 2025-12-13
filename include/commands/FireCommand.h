#pragma once

#include "ICommand.h"
#include "UObject.h"

class FireCommand : public ICommand
{
public:
    FireCommand(UObjectPtr obj) : _obj(obj) {}
    void exec() override
    {
        const auto ammo = _obj->getProperty<int>(UObject::AmmoProperty);
        if (ammo > 0)
        {
            _obj->setProperty(UObject::AmmoProperty, ammo - 1);
        }
    }

private:
    UObjectPtr _obj;
};
