#pragma once

#include "ICommand.h"

class DoubleRetryCommand : public ICommand
{
public:
    DoubleRetryCommand(ICommandUPtr cmd) : _cmd(std::move(cmd)) {}
    void exec()
    {
        _cmd->exec();
    }

private:
    ICommandUPtr _cmd;
};
