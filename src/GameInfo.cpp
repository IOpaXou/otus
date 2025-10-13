#include "GameInfo.h"
#include "json.hpp"

GameInfo GameInfo::fromJSON(const std::string& jsonStr)
{
    nlohmann::json j = nlohmann::json::parse(jsonStr);

    GameInfo gameInfo;
    gameInfo.owner = j["owner"];

    std::vector<std::string> participants;
    if (j.contains("participants") && j["participants"].is_array()) {
        for (const auto& participant : j["participants"]) {
            gameInfo.participants.push_back(participant.get<std::string>());
        }
    }

    return gameInfo;
}
