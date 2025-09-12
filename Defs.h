#pragma once

#include <any>
#include <functional>
#include <vector>

using Point = std::pair<int,int>;
using Vector = std::pair<int,int>;
using FuelUnit = double;

using AnyValue = std::any;
using Factory = std::function<std::any(const std::vector<AnyValue>&)>;
