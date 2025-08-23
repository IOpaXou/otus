#pragma once

#include "CommandException.h"
#include "ICommand.h"

class FailedCommand : public ICommand
{
public:
    FailedCommand(const std::string& msg) : _msg(msg) {}
    void exec() override
    {
        throw CommandException(_msg);
    }

private:
    std::string _msg;
};
