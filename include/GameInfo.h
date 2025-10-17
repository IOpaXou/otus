#pragma once

#include <string>
#include <vector>

struct GameInfo
{
    std::string gameId;
    std::vector<std::string> participants;
    std::string owner;

    static GameInfo fromJSON(const std::string& jsonStr);
};
