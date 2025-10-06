#pragma once

#include "ICommand.h"
#include "IQueue.h"

class QueuePushCommand : public ICommand
{
public:
    QueuePushCommand(IQueuePtr<ICommandPtr> cmdQueue, ICommandPtr cmd) : _cmdQueue(cmdQueue), _cmd(cmd) {}

    void exec() override
    {
        _cmdQueue->push(_cmd);
    }

private:
    IQueuePtr<ICommandPtr> _cmdQueue;
    ICommandPtr _cmd;
};
