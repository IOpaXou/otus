#pragma once

#include "IoC.h"

template <typename DependencyName, typename FactoryType>
ICommandPtr registerFactoryHelper(DependencyName&& name, FactoryType&& factory)
{
    std::string depName = std::forward<DependencyName>(name);
    Factory depFactory = std::forward<FactoryType>(factory);

    return IoC::Resolve<ICommandPtr>("IoC.Register", {depName, depFactory});
}

template <typename IoCHandlerName, typename IoCHandlerFunc>
void registerHandlerHelper(IoCHandlerName&& name, IoCHandlerFunc&& func)
{
    std::string handlerName = std::forward<IoCHandlerName>(name);
    IoC::IoCHandler handlerFunc = [handler = std::forward<IoCHandlerFunc>(func), handlerName]
    (const std::vector<AnyValue>& args) -> std::any
    {
        return handler(args);
    };

    IoC::Resolve<AnyValue>("IoC.Register.Handler", {handlerName, handlerFunc});
}
