#pragma once

#include "ICommand.h"
#include "IMovable.h"

class MoveCommand : public ICommand
{
public:
	explicit MoveCommand(IMovable& mObj) : _mObj(mObj) {}

	void exec() override
	{
		const auto startLoc = _mObj.getLocation();
		const auto velocity = _mObj.getVelocity();
		_mObj.setLocation({startLoc.first + velocity.first, startLoc.second + velocity.second});
	}

private:
	IMovable& _mObj;
};
