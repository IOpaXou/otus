#pragma once

#include "ICommand.h"
#include "UObject.h"

class FinishCommand : public ICommand
{
public:
    FinishCommand(const std::vector<AnyValue>& args) :
        _uObj(std::any_cast<UObjectPtr>(args[0])) {}

    void exec() override
    {
        _uObj->setProperty<bool>(UObject::FinishProperty, true);
    }

private:
    UObjectPtr _uObj;
};
