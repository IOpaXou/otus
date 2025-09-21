#pragma once

#include "ICommand.h"

class MacroCommand : public ICommand
{
public:
    explicit MacroCommand(std::vector<ICommandUPtr> commands) : _commands(std::move(commands)) {}
    void exec() override
    {
        for (auto& command : _commands)
        {
            command->exec();
        }
    }

private:
    std::vector<ICommandUPtr> _commands;
};
