#include "NormalState.h"
#include "MoveToState.h"

#include "HardStopCommand.h"
#include "MoveToCommand.h"

IStatePtr NormalState::handle(ICommandPtr cmd)
{
    cmd->exec();
    
    auto& cmdRef = *cmd;
    auto& cmdType = typeid(cmdRef);
    if (cmdType == typeid(HardStopCommand))
    {
        return IStatePtr();
    }
    else if (cmdType == typeid(MoveToCommand))
    {
        return std::make_shared<MoveToState>();
    }

    return shared_from_this();
}
