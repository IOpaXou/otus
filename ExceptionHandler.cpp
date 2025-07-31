#include "ExceptionHandler.h"

std::unordered_map<Key, Handler, KeyHash> ExceptionHandler::data = {};

ICommandUPtr ExceptionHandler::handle(ICommandUPtr cmd, const std::exception& ex)
{
    auto& cmdRef = *cmd;
    const std::type_index cmdTypeIndex = typeid(cmdRef);
    const std::type_index exTypeIndex = typeid(ex);

    auto handlerIt = data.find(Key(cmdTypeIndex, exTypeIndex));
    return handlerIt != data.end() ? handlerIt->second(std::move(cmd), ex)
                                    : nullptr;
}

void ExceptionHandler::registrate(const std::type_index& cmd, const std::type_index& ex, const Handler& h)
{
    data.emplace(Key(cmd, ex), h);
}
