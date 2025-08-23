#pragma once

#include "IMovable.h"

class MoveCommand
{
public:
	MoveCommand(IMovable& mObj) : _mObj(mObj) {}

	void exec()
	{
		const auto startLoc = _mObj.getLocation();
		const auto velocity = _mObj.getVelocity();
		_mObj.setLocation({startLoc.first + velocity.first, startLoc.second + velocity.second});
	}

private:
	IMovable& _mObj;
};
