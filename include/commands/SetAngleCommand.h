#pragma once

#include "ICommand.h"
#include "UObject.h"

class SetAngleCommand : public ICommand
{
public:
    SetAngleCommand(const std::vector<AnyValue>& args) :
        _uObj(std::any_cast<UObjectPtr>(args[0])), _angle(std::any_cast<double>(args[1])) {
    }

    void exec() override
    {
        _uObj->setProperty<double>(UObject::AngleProperty, _angle);
    }

private:
    UObjectPtr _uObj;
    double _angle;
};
