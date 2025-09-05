#include "RegisterCommand.h"
#include "IoC.h"

RegisterCommand::RegisterCommand(const std::string& dependency, const Factory& factory)
    : _dependency(std::move(dependency)), _factory(std::move(factory))
{
}

void RegisterCommand::exec()
{
    IoC::Resolve<void>("IoC.Register.Impl", {_dependency, _factory});
}
