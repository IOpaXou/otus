#include "ClearScopeCommand.h"
#include "IoC.h"

ClearScopeCommand::ClearScopeCommand(const std::string& scopeId) : _scopeId(scopeId)
{
}

void ClearScopeCommand::exec()
{
    IoC::Resolve<bool>("Scopes.Clear.Impl", {_scopeId});
}
