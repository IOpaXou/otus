#pragma once

#include "GameMessage.h"
#include "IoC.h"
#include "IQueue.h"
#include "UObject.h"

class InterpretCommand : public ICommand
{
public:
    InterpretCommand(const GameMessage& message, IQueuePtr<ICommandPtr> queue)
        : _message(message), _queue(queue) {}

    void exec() override
    {
        auto gameObject = resolveGameObject();
        if (!gameObject)
        {
            throw std::runtime_error("UObject not found " + _message.objectId);
        }

        setObjectProperties(gameObject);

        auto cmd = resolveGameCommand(gameObject);
        if (!cmd)
        {
            throw std::runtime_error("Command not found " + _message.commandId);
        }

        auto pushQueueCommand = IoC::Resolve<ICommandPtr>("Command.Queue.Push", {_queue, cmd});
        pushQueueCommand->exec();
    }

private:
    UObjectPtr resolveGameObject()
    {
        try
        {
            return IoC::Resolve<UObjectPtr>("GameObjects.Get", {_message.objectId});
        }
        catch (const std::exception& ex)
        {
            throw std::runtime_error("Failed to resolve UObject with id " + _message.objectId + " " + ex.what());
        }
    }

    void setObjectProperties(UObjectPtr uObj)
    {
        std::string commandKey = "Command." + _message.commandId + ".Properties.Set";
        try
        {
            _message.args.insert(_message.args.begin(), uObj);
            IoC::Resolve<void>(commandKey, _message.args);
        }
        catch (const std::exception& ex)
        {
            throw std::runtime_error("Failed to set properties for " + _message.commandId + " " + ex.what());
        }
    }

    ICommandPtr resolveGameCommand(UObjectPtr gameObject)
    {
        try
        {
            std::string commandKey = "Command." + _message.commandId;
            return IoC::Resolve<ICommandPtr>(commandKey, {gameObject});
        }
        catch (const std::exception& ex) {
            throw std::runtime_error("Failed to create command " + _message.commandId + " " + ex.what());
        }
    }

private:
    GameMessage _message;
    IQueuePtr<ICommandPtr> _queue;
};
