#pragma once

#include "IoCHelper.h"
#include "UObject.h"

#define DEFINE_ADAPTER(InterfaceName, AdapterName) \
    class AdapterName : public InterfaceName { \
    private: \
        UObjectPtr obj; \
    public: \
        AdapterName(UObjectPtr obj) : obj(obj) {} \
        static InterfaceName* Create(UObjectPtr obj) { \
            return new AdapterName(obj); \
        }

#define ADAPTER_GETTER(InterfaceName, MethodName) \
    auto MethodName() -> decltype(std::declval<InterfaceName>().MethodName()) override { \
        using ReturnType = decltype(std::declval<InterfaceName>().MethodName()); \
        std::vector<AnyValue> args = {AnyValue(obj)}; \
        return IoC::Resolve<ReturnType>(#InterfaceName ":" #MethodName, args); \
    }

#define ADAPTER_SETTER_REF(InterfaceName, MethodName, ParamType) \
    void MethodName(const ParamType& value) override { \
        std::vector<AnyValue> args = {AnyValue(obj), AnyValue(value)}; \
        IoC::Resolve<ICommandPtr>(#InterfaceName ":" #MethodName, args)->exec(); \
    }
#define ADAPTER_SETTER ADAPTER_SETTER_REF

#define ADAPTER_SETTER_VALUE(InterfaceName, MethodName, ParamType) \
    void MethodName(ParamType value) override { \
        std::vector<AnyValue> args = {AnyValue(obj), AnyValue(value)}; \
        IoC::Resolve<ICommandPtr>(#InterfaceName ":" #MethodName, args)->exec(); \
    }

#define ADAPTER_METHOD_VOID_NO_PARAMS(InterfaceName, MethodName) \
    void MethodName() override { \
        std::vector<AnyValue> args = {AnyValue(obj)}; \
        IoC::Resolve<ICommandPtr>(#InterfaceName ":" #MethodName, args)->exec(); \
    }

#define ADAPTER_METHOD ADAPTER_METHOD_VOID_NO_PARAMS

#define END_ADAPTER() \
    };

#define REGISTER_ADAPTER_FACTORY(InterfaceName, Adapter) \
inline void Register##InterfaceName##Adapter() { \
    auto dependencyName = std::string(#InterfaceName) + ".Adapter"; \
    auto factory = [](const std::vector<AnyValue>& args) { \
        auto obj = std::any_cast<UObjectPtr>(args[0]); \
        return std::static_pointer_cast<InterfaceName>(std::make_shared<Adapter>(obj)); \
    }; \
    registerFactoryHelper(dependencyName, factory)->exec(); \
}
