#pragma once

#include "ICommand.h"

class RetryCommand : public ICommand
{
public:
    RetryCommand(ICommandUPtr cmd) : _cmd(std::move(cmd)) {}
    void exec()
    {
        _cmd->exec();
    }

private:
    ICommandUPtr _cmd;
};
