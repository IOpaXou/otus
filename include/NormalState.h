#pragma once

#include "ICommand.h"
#include "IState.h"

class NormalState : public IState, public std::enable_shared_from_this<NormalState>
{
public:
    IStatePtr handle(ICommandPtr cmd) override;
};
