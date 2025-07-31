#pragma once

#include <functional>
#include <memory>
#include <queue>

class ICommand
{
public:
    virtual void exec() = 0;
};

using ICommandUPtr = std::unique_ptr<ICommand>;
using Handler = std::function<ICommandUPtr (ICommandUPtr, const std::exception&)>;
using CommandQueue = std::queue<ICommandUPtr>;
using CommandQueuePtr = std::shared_ptr<CommandQueue>;
