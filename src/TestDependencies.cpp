#include "IoCHelper.h"
#include "IQueue.h"
#include "MoveCommand.h"
#include "QueuePushCommand.h"
#include "TestMovableAdapter.h"
#include "UObject.h"

void registerHttpEndpointTestDependecies()
{
    RegisterIMovableTestMovableAdapter();

    registerFactoryHelper("GameObjects.Get", [](const std::vector<AnyValue>& args)
    {
        if (args.empty()) 
        {
            throw std::runtime_error("Object ID required");
        }

        try
        {
            auto objectId = std::any_cast<std::string>(args[0]);
        }
        catch (const std::bad_any_cast&)
        {
            throw std::runtime_error("Incorrect format for objectId");
        }

        return std::make_shared<UObject>();
    })->exec();

    registerFactoryHelper("Command.Move", [] (const std::vector<AnyValue>& args) -> ICommandPtr
    {
        if (args.empty())
        {
            throw std::runtime_error("UObject required for move command");
        }

        UObjectPtr uObj;
        try
        {
            uObj = std::any_cast<UObjectPtr>(args[0]);
        }
        catch (const std::bad_any_cast&)
        {
            throw std::runtime_error("Incorrect format for UObject");
        }

        auto mObj = IoC::Resolve<IMovablePtr>("IMovable.Adapter", {uObj});
        return std::make_shared<MoveCommand>(mObj);
    })->exec();
    
    registerFactoryHelper("Command.Move.Properties.Set", [] (const std::vector<AnyValue>& args)
    {
        if (args.size() != 5)
        {
            throw std::runtime_error("Move properties require UObject, locationX, locationY, velocityX, velocityY");
        }

        try
        {
            auto uObj = std::any_cast<UObjectPtr>(args[0]);
            double locationX = std::any_cast<double>(args[1]);
            double locationY = std::any_cast<double>(args[2]);
            double velocityX = std::any_cast<double>(args[3]);
            double velocityY = std::any_cast<double>(args[4]);
            
            uObj->setProperty(UObject::LocationProperty, Vector(locationX, locationY));
            uObj->setProperty(UObject::VelocityProperty, Vector(velocityX, velocityY));
        }
        catch (const std::bad_any_cast& ex)
        {
            throw std::runtime_error("Bad cast for MoveCommand properties " + std::string(ex.what()));
        }

        return AnyValue{};
    })->exec();

    registerFactoryHelper("Command.Queue.Push", [] (const std::vector<AnyValue>& args) -> ICommandPtr
    {
        if (args.size() != 2)
        {
            throw std::runtime_error("PushQueue requires queue and command");
        }

        try
        {
            auto queue = std::any_cast<IQueuePtr<ICommandPtr>>(args[0]);
            auto command = std::any_cast<ICommandPtr>(args[1]);

            return std::make_shared<QueuePushCommand>(queue, command);
        }
        catch (const std::bad_any_cast& ex)
        {
            throw std::runtime_error("Bad cast for PushQueue: " + std::string(ex.what()));
        }
    })->exec();
}
