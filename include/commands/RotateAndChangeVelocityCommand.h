#pragma once

#include "ChangeVelocityCommand.h"
#include "MacroCommand.h"
#include "MovableObject.h"
#include "RotatableObject.h"
#include "RotateCommand.h"

class RotateAndChangeVelocityCommand : public ICommand
{
public:
    explicit RotateAndChangeVelocityCommand(IRotatable* rObj, IMovable* mObj)
    {
        std::vector<ICommandUPtr> commands;
    	commands.emplace_back(new RotateCommand(*rObj));
	    commands.emplace_back(new ChangeVelocityCommand(rObj, mObj));

        command = std::make_unique<MacroCommand>(std::move(commands));
    }

    void exec() override
    {
        command->exec();
    }

private:
    ICommandUPtr command;
};
