#pragma once

#include "Defs.h"

#include <json_fwd.hpp>
#include <string>

struct GameMessage
{
    std::string gameId;
    std::string objectId;
    std::string commandId;
    std::vector<AnyValue> args;

    static GameMessage fromJSON(const std::string& jsonStr);
};
