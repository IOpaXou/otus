#pragma once

#include "IMovable.h"

class Move
{
public:
	Move(IMovable& mObj) : _mObj(mObj) {}
	
	void exec()
	{
		const auto startLoc = _mObj.getLocation();
		const auto velocity = _mObj.getVelocity();
		_mObj.setLocation({startLoc.first + velocity.first, startLoc.second + velocity.second});
	}

private:
	IMovable& _mObj;
};
