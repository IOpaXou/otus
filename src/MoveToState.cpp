#include "MoveToState.h"

#include "IQueue.h"
#include "NormalState.h"

#include "IoC.h"
#include "HardStopCommand.h"
#include "RunCommand.h"

IStatePtr MoveToState::handle(ICommandPtr cmd)
{
    auto& cmdRef = *cmd;
    auto& cmdType = typeid(cmdRef);
    if (cmdType == typeid(HardStopCommand))
    {
        return IStatePtr();
    }
    else if (cmdType == typeid(RunCommand))
    {
        return std::make_shared<NormalState>();
    }
    else
    {
        auto backupQueue = IoC::Resolve<IQueuePtr<ICommandPtr>>("Queue.Backup", {});
        if (backupQueue)
        {
            backupQueue->push(cmd);
        }
        else
        {
        }
        return shared_from_this();
    }
}
