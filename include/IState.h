#pragma once

#include "ICommand.h"

class IState;
using IStatePtr = std::shared_ptr<IState>;

class IState
{
public:
    virtual IStatePtr handle(ICommandPtr) = 0;
};
