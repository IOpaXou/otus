#include "GameMessage.h"
#include "json.hpp"

#include <stdexcept>

namespace
{
    class GameMessageParseError : public std::runtime_error
    {
    public:
        GameMessageParseError(const std::string& msg) : std::runtime_error(msg) {}
    };

    std::vector<AnyValue> parseArray(const nlohmann::json& j)
    {
        std::vector<AnyValue> args;

        for (const auto& elem : j)
        {
            if (elem.is_number_integer()) {
                args.push_back(elem.get<int>());
            } else if (elem.is_number_float()) {
                args.push_back(elem.get<double>());
            } else if (elem.is_boolean()) {
                args.push_back(elem.get<bool>());
            } else if (elem.is_string()) {
                args.push_back(elem.get<std::string>());
            } else if (elem.is_null()) {
                args.push_back(nullptr);
            } else if (elem.is_array()) {
                args.push_back(parseArray(elem));
            }
            else {
                throw GameMessageParseError("Incorrect argument");
            }
        }
        return args;
    }

    std::vector<AnyValue> parseObject(const nlohmann::json& j)
    {
        std::vector<AnyValue> args;

        for (auto it = j.begin(); it != j.end(); ++it)
        {
            std::pair<std::string, AnyValue> pair;
            pair.first = it.key();
            
            if (it.value().is_string()) {
                pair.second = it.value().get<std::string>();
            } else if (it.value().is_number_integer()) {
                pair.second = it.value().get<int>();
            } else if (it.value().is_number_float()) {
                pair.second = it.value().get<double>();
            } else if (it.value().is_boolean()) {
                pair.second = it.value().get<bool>();
            } else {
                pair.second = it.value().dump();
            }
            
            args.push_back(pair);
        }
        
        return args;
    }
}

GameMessage GameMessage::fromJSON(const std::string& jsonStr)
{
    try
    {
        nlohmann::json j = nlohmann::json::parse(jsonStr);

        GameMessage msg;

        msg.gameId = j["gameId"];
        msg.objectId = j["objectId"];
        msg.commandId = j["commandId"];

        if (j.contains("jwt"))
        {
            msg.jwt = j["jwt"];
        }

        if (msg.gameId.empty() || msg.objectId.empty() || msg.commandId.empty())
        {
            throw GameMessageParseError("Missing arguments");
        }

        if (j.contains("args"))
        {
            if (j["args"].is_array())
            {
                msg.args = parseArray(j["args"]);
            }
            else if (j["args"].is_object())
            {
                msg.args = parseObject(j["args"]);
            }
            else
            {
                throw GameMessageParseError("Field 'args' must be array or object");
            }
        }
        return msg;
    }
    catch (const nlohmann::json::exception& e)
    {
        throw GameMessageParseError("JSON parse error " + std::string(e.what()));
    }
}
