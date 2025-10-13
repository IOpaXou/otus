#pragma once

#include <string>

struct AuthRequest
{
    std::string gameId;
    std::string userId;

    static AuthRequest fromJSON(const std::string& jsonStr);
};
