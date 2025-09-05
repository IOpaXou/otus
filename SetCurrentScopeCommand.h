#pragma once

#include "ICommand.h"
#include <string>

class SetCurrentScopeCommand : public ICommand
{
public:
    SetCurrentScopeCommand(const std::string& scopeId);
    void exec() override;

private:
    std::string _scopeId;
};
