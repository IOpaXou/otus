#pragma once

#include "Defs.h"
#include "ICommand.h"

#include <mutex>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>

class IoC
{
public:
    using Scope = std::unordered_map<std::string, Factory>;
    using ScopePtr = std::shared_ptr<Scope>;
    using ScopeMap = std::unordered_map<std::string, ScopePtr>;

    using IoCHandler = std::function<AnyValue(const std::vector<AnyValue>&)>;
    using IoCHandlerMap = std::unordered_map<std::string, IoCHandler>;

public:
    template<typename T>
    static T Resolve(const std::string& key, const std::vector<AnyValue>& args = {})
    {
        static std::once_flag initFlag;
        std::call_once(initFlag, init);

        {
            auto handler = _handlers.find(key);
            if (handler != _handlers.end()) {
                auto result = handler->second(args);
                return convertReturnType<T>(result);
            }
        }

        auto scope = getCurrentScope();
        auto it = scope->find(key);
        if (it != scope->end())
        {
            auto factory = it->second;
            try
            {
                auto result = factory(args);
                if (!result.has_value())
                {
                    if constexpr (std::is_default_constructible_v<T>)
                    {
                        return T{};
                    }
                    else
                    {
                        throw std::runtime_error("Cannot convert empty value to non-default-constructible type for dependency: " + key);
                    }
                }

                if constexpr (std::is_same_v<T, void>)
                {
                    return;
                }
                else
                {
                    try
                    {
                        return std::any_cast<T>(result);
                    }
                    catch (const std::bad_any_cast&)
                    {
                        try
                        {
                            return std::any_cast<T&>(result);
                        }
                        catch (const std::bad_any_cast&)
                        {
                            throw std::runtime_error("Failed to cast for key " + key);
                        }
                    }
                }
            }
            catch (const std::bad_any_cast&)
            {
                throw std::runtime_error("Type mismatch for dependency: " + key);
            }
        }
        else
        {
            throw std::runtime_error("Dependency not found: " + key);
        }
    }

private:
    IoC() = delete;

    static void init();
    static void RegisterHandler(const std::string& key, IoCHandler handler);

    static ICommandPtr createRegisterCommand(const std::vector<AnyValue>& args);
    static ICommandPtr createScopeCommand(const std::vector<AnyValue>& args);
    static ICommandPtr createSetCurrentScopeCommand(const std::vector<AnyValue>& args);
    static ICommandPtr createClearScopeCommand(const std::vector<AnyValue>& args);

    template <typename T> 
    static T convertReturnType(AnyValue value = {})
    {
        if constexpr (std::is_same_v<T, void>)
        {
            return;
        }
        else if (!value.has_value())
        {
            if constexpr (std::is_default_constructible_v<T>)
            {
                return T{};
            }
        }
        else
        {
            try
            {
                return std::any_cast<T>(value);
            }
            catch (const std::bad_any_cast&)
            {
                throw std::runtime_error("Can't convert return type on convertReturnType");
            }
        }
    }

    static void registerImpl(const std::vector<AnyValue>& args);
    static bool createScopeImpl(const std::vector<AnyValue>& args);
    static bool setCurrentScopeImpl(const std::vector<AnyValue>& args);
    static bool clearScopeImpl(const std::vector<AnyValue>& args);
    static ScopePtr getCurrentScope();

private:
    static ScopeMap _scopes;
    static std::mutex _scopesMutex;
    
    static IoCHandlerMap _handlers;
    static std::mutex _handlersMutex;

    static thread_local ScopePtr _currentScope;
    static thread_local bool _scopeInitialized;
};
