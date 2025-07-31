#include "ExceptionHandler.h"

ICommandUPtr ExceptionHandler::handle(ICommandUPtr cmd, const std::exception& ex)
{
    auto& cmdRef = *cmd;
    const std::type_index cmdTypeIndex = typeid(cmdRef);
    const std::type_index exTypeIndex = typeid(ex);

    auto handlerIt = getData().find(Key(cmdTypeIndex, exTypeIndex));
    return handlerIt != getData().end() ? handlerIt->second(std::move(cmd), ex)
                                    : nullptr;
}

void ExceptionHandler::registrate(const std::type_index& cmd, const std::type_index& ex, const Handler& h)
{
    getData().emplace(Key(cmd, ex), h);
}
