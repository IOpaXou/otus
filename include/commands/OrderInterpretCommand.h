#pragma once

#include "IoC.h"
#include "PlayerScopeManager.h"
#include "UObject.h"

class OrderInterpretCommand : public ICommand
{
public:
    OrderInterpretCommand(PlayerScopeManagerPtr scopeManager, UObjectPtr order, std::string playerId) : _playerScopeManager(scopeManager), _order(order), _playerId(playerId) {}

    void exec() override
    {
        _playerScopeManager->setCurrentPlayerScope(_playerId);

        std::string objectId;
        try
        {
            objectId = _order->getProperty<std::string>("id");
        }
        catch (const std::exception& ex)
        {
            objectId = "";
        }

        UObjectPtr gameObject;
        if (!objectId.empty())
        {
            gameObject = IoC::Resolve<UObjectPtr>("GameObjects.Get", {objectId});
        }

        auto actionId = _order->getProperty<std::string>("action");
        auto isCmdAvailable = IoC::Resolve<bool>("Command.CheckAvailable", {actionId});

        _playerScopeManager->setDefaultScope();

        if (!isCmdAvailable && !gameObject) // system command
        {
            isCmdAvailable = IoC::Resolve<bool>("Command.CheckAvailable", {actionId});
        }

        if (isCmdAvailable && (gameObject || objectId.empty()))
        {
            auto commandKey = "Command." + actionId;
            auto orderCmd = IoC::Resolve<ICommandPtr>(commandKey, {gameObject});
            orderCmd->exec();
        }
        else
        {
            throw std::runtime_error("Incorrect order " + objectId + " " + actionId);
        }
    }

private:
    PlayerScopeManagerPtr _playerScopeManager;
    UObjectPtr _order;
    std::string _playerId;
};
