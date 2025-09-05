#pragma once

#include "Defs.h"
#include "ICommand.h"
#include <string>

class RegisterCommand : public ICommand
{
public:
    RegisterCommand(const std::string& dependency, const Factory& factory);
    void exec() override;

private:
    std::string _dependency;
    Factory _factory;
};
