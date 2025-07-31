#pragma once

#include "ICommand.h"

#include <exception>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>

using Key = std::pair<std::type_index, std::type_index>;

struct KeyHash
{
    size_t operator()(const Key& key) const
    {
        return std::hash<std::type_index>()(key.first) ^ (
            std::hash<std::type_index>()(key.second) << 1);
    }
};

class ExceptionHandler
{
public:
    static ICommandUPtr handle(ICommandUPtr cmd, const std::exception& ex);
    static void registrate(const std::type_index& cmd, const std::type_index& ex, const Handler& h);

private:
    static std::unordered_map<Key, Handler, KeyHash> data;
};
