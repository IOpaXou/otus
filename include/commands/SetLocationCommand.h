#pragma once

#include "ICommand.h"
#include "UObject.h"

class SetLocationCommand : public ICommand
{
public:
    SetLocationCommand(const std::vector<AnyValue>& args) :
        _uObj(std::any_cast<UObjectPtr>(args[0])), _loc(std::any_cast<Point>(args[1])) {
    }

    void exec() override
    {
        _uObj->setProperty<Point>(UObject::LocationProperty, _loc);
    }

private:
    UObjectPtr _uObj;
    Point _loc;
};
