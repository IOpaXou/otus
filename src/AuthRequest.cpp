#include "AuthRequest.h"
#include "json.hpp"

AuthRequest AuthRequest::fromJSON(const std::string& jsonStr)
{
    nlohmann::json j = nlohmann::json::parse(jsonStr);

    AuthRequest authReq;
    authReq.userId = j["userId"];
    authReq.gameId = j["gameId"];

    return authReq;
}
