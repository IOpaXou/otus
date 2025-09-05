#pragma once

#include "ICommand.h"
#include <string>

class CreateScopeCommand : public ICommand
{
public:
    CreateScopeCommand(const std::string& scopeId);
    void exec() override;

private:
    std::string _scopeId;
};
