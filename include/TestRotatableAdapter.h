#pragma once

#include "AdapterGenerator.h"
#include "IRotatable.h"

DEFINE_ADAPTER(IRotatable, TestRotatableAdapter)
	ADAPTER_GETTER(IRotatable, getAngle)
	ADAPTER_GETTER(IRotatable, getAngularVelocity)
	ADAPTER_SETTER_VALUE(IRotatable, setAngle, double)
END_ADAPTER()

REGISTER_ADAPTER_FACTORY(IRotatable, TestRotatableAdapter)

using TestRotatablePtr = std::shared_ptr<TestRotatableAdapter>;
