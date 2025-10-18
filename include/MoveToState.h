#pragma once

#include "ICommand.h"
#include "IState.h"

class MoveToState : public IState, public std::enable_shared_from_this<MoveToState>
{
public:
    IStatePtr handle(ICommandPtr cmd) override;
};
