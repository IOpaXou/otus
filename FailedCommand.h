#pragma once

#include "CustomException.h"
#include "ICommand.h"

class FailedCommand : public ICommand
{
public:
    FailedCommand(const std::string& msg) : _msg(msg) {}
    void exec() override
    {
        throw CustomException(_msg);
    }

private:
    std::string _msg;
};
