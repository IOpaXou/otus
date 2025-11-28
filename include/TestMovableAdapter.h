#pragma once

#include "AdapterGenerator.h"
#include "IMovable.h"

DEFINE_ADAPTER(IMovable, TestMovableAdapter)
	ADAPTER_GETTER(IMovable, getLocation)
	ADAPTER_GETTER(IMovable, getVelocity)
	ADAPTER_SETTER(IMovable, setLocation, Point)
	ADAPTER_SETTER(IMovable, setVelocity, Vector)
	ADAPTER_METHOD(IMovable, finish)
END_ADAPTER()

REGISTER_ADAPTER_FACTORY(IMovable, TestMovableAdapter)

using TestMovableAdapterPtr = std::shared_ptr<TestMovableAdapter>;
