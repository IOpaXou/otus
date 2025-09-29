#pragma once

#include "ICommand.h"

using TestFunc = std::function<void()>;

class TestCommand : public ICommand
{
public:
    TestCommand(TestFunc func) : _func(std::move(func)) {}
    void exec() override
    {
        if (_func)
        {
            _func();
        }
    }

private:
    TestFunc  _func;
};
