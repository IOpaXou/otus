#pragma once

#include "ICommand.h"
#include <string>

class ClearScopeCommand : public ICommand
{
public:
    ClearScopeCommand(const std::string& scopeId);
    void exec() override;

private:
    std::string _scopeId;
};
