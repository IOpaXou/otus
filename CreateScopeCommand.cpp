#include "CreateScopeCommand.h"
#include "IoC.h"

CreateScopeCommand::CreateScopeCommand(const std::string& scopeId) : _scopeId(scopeId)
{
}

void CreateScopeCommand::exec()
{
    IoC::Resolve<bool>("Scopes.New.Impl", {_scopeId});
}
