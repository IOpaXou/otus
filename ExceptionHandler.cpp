#include "ExceptionHandler.h"
#include <iostream>

ICommandUPtr ExceptionHandler::handle(ICommandUPtr cmd, const std::exception& ex)
{
    std::cerr << "\n=== Handling exception ===\n";
    std::cerr << "Command type: " << typeid(*cmd).name() << "\n";
    std::cerr << "Exception type: " << typeid(ex).name() << "\n";

    auto& data = getData();
    std::cerr << "Registered handlers (" << data.size() << "):\n";
    for (const auto& [key, _] : data) {
        std::cerr << "- " << key.first.name() << " -> " << key.second.name() << "\n";
    }
    
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
