#pragma once

#include "ICommand.h"
#include "UObject.h"

#include <iostream>

class SaveCommand : public ICommand
{
public:
    SaveCommand(UObjectPtr obj) : _obj(obj) {}
    void exec() override
    {
        std::cout << "Saved" << std::endl;
    }

private:
    UObjectPtr _obj;
};
