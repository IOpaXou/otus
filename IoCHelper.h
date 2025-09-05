#pragma once

#include "ICommand.h"
#include "IoC.h"

#include <utility>

template <typename DependencyName, typename FactoryType>
ICommandPtr registerFactoryHelper(DependencyName&& name, FactoryType&& factory)
{
    std::string depName = std::forward<DependencyName>(name);
    Factory depFactory = std::forward<FactoryType>(factory);

    return IoC::Resolve<ICommandPtr>("IoC.Register", {depName, depFactory});
}

template <typename IoCHandlerName, typename IoCHandlerType>
ICommandPtr registerHandlerHelper(IoCHandlerName&& name, IoCHandlerType&& type)
{
    std::string handlerName = std::forward<IoCHandlerName>(name);
    IoC::IoCHandler handlerType = std::forward<IoCHandlerType>(type);

    return IoC::Resolve<ICommandPtr>("IoC.Register.Handler", {handlerName, handlerType});
}
