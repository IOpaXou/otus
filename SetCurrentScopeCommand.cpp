#include "SetCurrentScopeCommand.h"
#include "IoC.h"

SetCurrentScopeCommand::SetCurrentScopeCommand(const std::string& scopeId) : _scopeId(scopeId)
{
}

void SetCurrentScopeCommand::exec()
{
    IoC::Resolve<bool>("Scopes.Current.Impl", {_scopeId});
}
