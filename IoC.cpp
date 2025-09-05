#include "IoC.h"

#include "ClearScopeCommand.h"
#include "CreateScopeCommand.h"
#include "RegisterCommand.h"
#include "SetCurrentScopeCommand.h"

IoC::ScopeMap IoC::_scopes;
std::mutex IoC::_scopesMutex;

IoC::IoCHandlerMap IoC::_handlers;
std::mutex IoC::_handlersMutex;

thread_local IoC::ScopePtr IoC::_currentScope = nullptr;
thread_local bool IoC::_scopeInitialized = false;

void IoC::init()
{   
    RegisterHandler("IoC.Register.Handler", [](const std::vector<AnyValue>& args) {
        if (args.size() < 2) {
            throw std::runtime_error("IoC.Register.Handler requires 2 arguments: key and handler");
        }
        auto key = std::any_cast<std::string>(args[0]);
        auto handler = std::any_cast<IoCHandler>(args[1]);
        RegisterHandler(key, handler);
        return AnyValue();
    });
    RegisterHandler("IoC.Register", [](const std::vector<AnyValue>& args) {
        return AnyValue(createRegisterCommand(args));
    });
    
    RegisterHandler("Scopes.New", [](const std::vector<AnyValue>& args) {
        return AnyValue(createScopeCommand(args));
    });
    
    RegisterHandler("Scopes.Current", [](const std::vector<AnyValue>& args) {
        return AnyValue(createSetCurrentScopeCommand(args));
    });
    
    RegisterHandler("Scopes.Clear", [](const std::vector<AnyValue>& args) {
        return AnyValue(createClearScopeCommand(args));
    });
    
    RegisterHandler("IoC.Register.Impl", [](const std::vector<AnyValue>& args) {
        registerImpl(args);
        return AnyValue();
    });
    
    RegisterHandler("Scopes.New.Impl", [](const std::vector<AnyValue>& args) {
        return AnyValue(createScopeImpl(args));
    });
    
    RegisterHandler("Scopes.Current.Impl", [](const std::vector<AnyValue>& args) {
        return AnyValue(setCurrentScopeImpl(args));
    });
    
    RegisterHandler("Scopes.Clear.Impl", [](const std::vector<AnyValue>& args) {
        return AnyValue(clearScopeImpl(args));
    });
}

void IoC::RegisterHandler(const std::string& key, IoCHandler handler)
{
    std::lock_guard<std::mutex> lock(_handlersMutex);
    _handlers[key] = std::move(handler);
}

ICommandPtr IoC::createRegisterCommand(const std::vector<AnyValue>& args)
{
    if (args.size() != 2)
    {
        throw std::invalid_argument("IoC.Register requires 2 arguments");
    }

    try
    {
        std::string dependencyName = std::any_cast<std::string>(args.at(0));
        Factory factory = std::any_cast<Factory>(args.at(1));

        return std::make_shared<RegisterCommand>(dependencyName, factory);
    }
    catch (const std::bad_any_cast&)
    {
        throw std::invalid_argument("IoC.Register arguments have to be string and Factory");
    }
}

ICommandPtr IoC::createScopeCommand(const std::vector<AnyValue>& args)
{
    if (args.empty()) 
    {
        throw std::invalid_argument("Scope ID required");
    }
    std::string scopeId = std::any_cast<std::string>(args[0]);
    return std::make_shared<CreateScopeCommand>(scopeId);
}

ICommandPtr IoC::createSetCurrentScopeCommand(const std::vector<AnyValue>& args)
{
    if (args.empty()) 
    {
        throw std::invalid_argument("Scope ID required");
    }
    std::string scopeId = std::any_cast<std::string>(args[0]);
    return std::make_shared<SetCurrentScopeCommand>(scopeId);
}

ICommandPtr IoC::createClearScopeCommand(const std::vector<AnyValue>& args)
{
    if (args.empty()) 
    {
        throw std::invalid_argument("Scope ID required");
    }
    std::string scopeId = std::any_cast<std::string>(args[0]);
    return std::make_shared<ClearScopeCommand>(scopeId);
}

void IoC::registerImpl(const std::vector<AnyValue>& args)
{
    if (args.size() != 2) 
    {
        throw std::invalid_argument("IoC.Register.Impl requires 2 arguments");
    }
    std::string dependencyName = std::any_cast<std::string>(args[0]);
    Factory factory = std::any_cast<Factory>(args[1]);
    
    auto scope = getCurrentScope();
    (*scope)[dependencyName] = factory;
}

bool IoC::createScopeImpl(const std::vector<AnyValue>& args)
{
    if (args.empty()) 
    {
        throw std::invalid_argument("Scope ID required");
    }
    std::string scopeId = std::any_cast<std::string>(args[0]);

    std::lock_guard<std::mutex> lock(_scopesMutex);

    if (_scopes.find(scopeId) == _scopes.end())
    {
        _scopes[scopeId] = std::make_shared<Scope>();
        return true;
    }
    return false;
}

bool IoC::setCurrentScopeImpl(const std::vector<AnyValue>& args)
{
    if (args.empty())
    {
        throw std::invalid_argument("Scope ID required");
    }
    std::string scopeId = std::any_cast<std::string>(args.at(0));

    std::lock_guard<std::mutex> lock(_scopesMutex);

    auto it = _scopes.find(scopeId);
    if (it != _scopes.end())
    {
        _currentScope = it->second;
        return true;
    }
    return false;
}

bool IoC::clearScopeImpl(const std::vector<AnyValue>& args)
{
    if (args.empty())
    {
        throw std::invalid_argument("Scope ID required");
    }
    std::string scopeId = std::any_cast<std::string>(args.at(0));

    std::lock_guard<std::mutex> lock(_scopesMutex);

    auto it = _scopes.find(scopeId);
    if (it != _scopes.end())
    {
        _scopes.erase(it);
        if (_currentScope == it->second)
        {
            _currentScope = _scopes["default"];
        }
        return true;
    }

    return false;
}

IoC::ScopePtr IoC::getCurrentScope()
{
    if (!_scopeInitialized)
    {
        std::lock_guard<std::mutex> lock(_scopesMutex);

        if (_scopes.find("default") == _scopes.end())
        {
            _scopes["default"] = std::make_shared<Scope>();
        }
        _currentScope = _scopes["default"];
        _scopeInitialized = true;
    }

    return _currentScope;
}
