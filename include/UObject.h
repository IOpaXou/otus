#pragma once

#include "Defs.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

class UObject
{
public:
	template <typename T>
	T getProperty(const std::string& key)
	{
		auto it = _properties.find(key);
		if (it != _properties.end())
		{
			return std::any_cast<T>(it->second);
		}
		throw std::runtime_error("There is no property " + key);
	}

	template <typename T>
	void setProperty(const std::string& key, T value)
	{
		_properties[key] = value;
	}

	static inline std::string LocationProperty = "location";
	static inline std::string VelocityProperty = "velocity";
	static inline std::string FinishProperty = "finish";

private:
	std::unordered_map<std::string, AnyValue> _properties;
};

using UObjectPtr = std::shared_ptr<UObject>;
