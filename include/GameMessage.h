#pragma once

#include "Defs.h"

#include <string>

struct GameMessage
{
    std::string gameId;
    std::string objectId;
    std::string commandId;
    std::vector<AnyValue> args;
    std::string jwt;

    static GameMessage fromJSON(const std::string& jsonStr);
};
