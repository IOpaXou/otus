#include "CommandException.h"
#include "ExceptionHandler.h"
#include "DoubleRetryCommand.h"
#include "FailedCommand.h"
#include "ICommand.h"
#include "LogCommand.h"
#include "RetryCommand.h"

#include <exception>
#include <fstream>
#include <memory>

#include <gtest/gtest.h>

namespace
{
	const std::string customExText = "Alarm";

	auto logHandler = [](ICommandUPtr cmd, const std::exception& ex) -> ICommandUPtr
	{
		return std::make_unique<LogCommand>(ex);
	};

	auto retryHandler = [](ICommandUPtr cmd, const std::exception& ex) -> ICommandUPtr
	{
		return std::make_unique<RetryCommand>(std::move(cmd));
	};

	auto doubleRetryHandler = [](ICommandUPtr cmd, const std::exception& ex) -> ICommandUPtr
	{
		return std::make_unique<DoubleRetryCommand>(std::move(cmd));
	};

	void checkLog(const std::string& txt)
	{
		std::ifstream fin;
		fin.open(LogCommand::LogPath(), std::ios::in);
		ASSERT_TRUE(fin.is_open());

		std::string logText;
		fin >> logText;

		ASSERT_EQ(txt, logText);
	}

	class LogExceptionHandler
	{
	public:
		LogExceptionHandler(CommandQueuePtr cmdQueue) : _cmdQueue(cmdQueue) {}
		void handle(ICommandUPtr cmd, const std::exception& ex)
		{
			ICommandUPtr logCommand = std::make_unique<LogCommand>(ex);
			_cmdQueue->push(std::move(logCommand));
		}

	private:
		CommandQueuePtr _cmdQueue;
	};

	class RetryExceptionHandler
	{
	public:
		RetryExceptionHandler(CommandQueuePtr cmdQueue) : _cmdQueue(cmdQueue) {}
		void handle(ICommandUPtr cmd, const std::exception& ex)
		{
			ICommandUPtr retryCommand = std::make_unique<RetryCommand>(std::move(cmd));
			_cmdQueue->push(std::move(retryCommand));
		}

	private:
		CommandQueuePtr _cmdQueue;
	};

	class LogAndRetryExceptionHandler
	{
	public:
		LogAndRetryExceptionHandler() {}
		void handle(ICommandUPtr cmd, const std::exception& ex)
		{
			auto& cmdRef = *cmd;
			ICommandUPtr cmdPtr;
			if (retryApplied)
			{
				cmdPtr = std::make_unique<LogCommand>(ex);
			}
			else
			{
				cmdPtr = std::make_unique<RetryCommand>(std::move(cmd));
				retryApplied = true;
			}
			cmdPtr->exec();
		}
	private:
		bool retryApplied = false;
	};
}

TEST(ExceptionTestSuite, LogCommand)
{
	CommandException customEx(customExText);
	ICommandUPtr logCmd = std::make_unique<LogCommand>(customEx);
	logCmd->exec();

	checkLog(customExText);
}

TEST(ExceptionTestSuite, LogHandler)
{
	const std::string failedCmdText = "FailedLogHandlerTest";

	CommandQueuePtr cmdQueue = std::make_shared<CommandQueue>();

	CommandException customEx(customExText);
	ICommandUPtr logCmd = std::make_unique<LogCommand>(customEx);

	LogExceptionHandler logHandler(cmdQueue);

	ASSERT_TRUE(cmdQueue->empty());

	ICommandUPtr failedCmd = std::make_unique<FailedCommand>(failedCmdText);
	try
	{
		failedCmd->exec();
		FAIL() << "unreachable";
	}
	catch (const std::exception& ex)
	{
		logHandler.handle(std::move(failedCmd), ex);
	}

	ASSERT_EQ(cmdQueue->size(), 1);
	
	auto cmd = std::move(cmdQueue->front());
	auto& cmdRef = *cmd;
	ASSERT_EQ(typeid(cmdRef), typeid(LogCommand));
}

TEST(ExceptionTestSuite, RetryCommand)
{
	const std::string failedCmdText = "FailedRetryCommandTest";

	ICommandUPtr failedCmd = std::make_unique<FailedCommand>(failedCmdText);
	RetryCommand retryCmd(std::move(failedCmd));

	try
	{
		retryCmd.exec();
		FAIL() << "unreachable";
	}
	catch (const std::exception& ex)
	{
		ASSERT_EQ(failedCmdText, ex.what());
	}
}

TEST(ExceptionTestSuite, RetryHandler)
{
	const std::string failedCmdText = "FailedRetryHandlerTest";

	CommandQueuePtr cmdQueue = std::make_shared<CommandQueue>();
	RetryExceptionHandler retryHandler(cmdQueue);

	ASSERT_TRUE(cmdQueue->empty());

	ICommandUPtr failedCmd = std::make_unique<FailedCommand>(failedCmdText);
	try
	{
		failedCmd->exec();
		FAIL() << "unreachable";
	}
	catch (const std::exception& ex)
	{
		retryHandler.handle(std::move(failedCmd), ex);
	}

	ASSERT_EQ(cmdQueue->size(), 1);

	auto cmd = std::move(cmdQueue->front());
	auto& cmdRef = *cmd;
	ASSERT_EQ(typeid(cmdRef), typeid(RetryCommand));

	try
	{
		cmd->exec();
		FAIL() << "unreachable";
	}
	catch (const std::exception& ex)
	{
		ASSERT_EQ(failedCmdText, ex.what());
	}
}

TEST(ExceptionTestSuite, LogAndRetryHandler)
{
	const std::string failedCmdText = "FailedLogAndRetryHandlerTest";

	LogAndRetryExceptionHandler logAndRetryHandler;

	ICommandUPtr failedCmd = std::make_unique<FailedCommand>(failedCmdText);
	try
	{
		failedCmd->exec();
		FAIL() << "unreachable";
	}
	catch (const std::exception& ex)
	{
		try
		{
			logAndRetryHandler.handle(std::move(failedCmd), ex);
			FAIL() << "unreachable";
		}
		catch (const std::exception& ex)
		{
			logAndRetryHandler.handle(std::move(failedCmd), ex);
		}
	}

	checkLog(failedCmdText);
}

TEST(ExceptionTestSuite, DoubleRetryAndLogHandler)
{
	ExceptionHandler::registrate(typeid(FailedCommand), typeid(CommandException), retryHandler);
	ExceptionHandler::registrate(typeid(RetryCommand), typeid(CommandException), doubleRetryHandler);
	ExceptionHandler::registrate(typeid(DoubleRetryCommand), typeid(CommandException), logHandler);

	const std::string failedCmdText = "DoubleRetryAndLogHandlerTest";

	ICommandUPtr failedCmd = std::make_unique<FailedCommand>(failedCmdText);
	auto& failedCmdRef = *failedCmd;
	ASSERT_EQ(typeid(failedCmdRef), typeid(FailedCommand));

	try
	{
		failedCmd->exec();
		FAIL() << "unreachable";
	}
	catch (const std::exception& ex)
	{
		failedCmd = ExceptionHandler::handle(std::move(failedCmd), ex);
		auto& failedCmdRef = *failedCmd;
		ASSERT_EQ(typeid(failedCmdRef), typeid(RetryCommand));
		try
		{
			failedCmd->exec();
			FAIL() << "unreachable";
		}
		catch (const std::exception& ex)
		{
			failedCmd = ExceptionHandler::handle(std::move(failedCmd), ex);
			auto& failedCmdRef = *failedCmd;
			ASSERT_EQ(typeid(failedCmdRef), typeid(DoubleRetryCommand));
			try
			{
				failedCmd->exec();
				FAIL() << "unreachable";
			}
			catch (const std::exception& ex)
			{
				failedCmd = ExceptionHandler::handle(std::move(failedCmd), ex);
				auto& failedCmdRef = *failedCmd;
				ASSERT_EQ(typeid(failedCmdRef), typeid(LogCommand));
				try
				{
					failedCmd->exec();
				}
				catch (const std::exception& ex)
				{
					FAIL() << "unreachable";
				}
			}
		}
	}

	checkLog(failedCmdText);
}

#include "IMovable.h"
#include "MoveCommand.h"
#include "MovableObject.h"
#include "RotateCommand.h"
#include "RotatableObject.h"

#include <climits>
#include <cmath>
#include <exception>
#include <gtest/gtest.h>

constexpr Point InitialPos {12, 5};
constexpr Vector InitialVelocity {-7, 3};

class MovableButNotGetLocatable : public IMovable
{
public:
	Point getLocation() override {
		throw std::runtime_error("Can't get location");
	}
	Vector getVelocity() override {return {};}
	void setLocation(const Point&) override {}
	void setVelocity(const Vector&) override {}
};

class MovableButNotGetVelocitable : public IMovable
{
public:
	Point getLocation() override {return {};}
	Vector getVelocity() override {
		throw std::runtime_error("Can't get velocity");
	}
	void setLocation(const Point&) override {}
	void setVelocity(const Vector&) override {}
};

class MovableButNotSetLocatable : public IMovable
{
public:
	Point getLocation() override {return {};}
	Vector getVelocity() override {return {};}
	void setLocation(const Point&) override {
		throw std::runtime_error("Can't set location");
	}
	void setVelocity(const Vector&) override {}
};

const auto EPSILON = 1e-9;

std::vector<double> solve(double a, double b, double c)
{
	if (!std::isfinite(a) || !std::isfinite(b) || !std::isfinite(c))
	{
		throw std::invalid_argument("Invalid argument");
	}

	if (std::abs(a) < EPSILON)
	{
		throw std::invalid_argument("Zero A");
	}

	const auto D = b*b - 4*a*c;
	if (D > EPSILON)
	{
		const auto x1 = (-b + std::sqrt(D)) / 2*a;
		const auto x2 = (-b - std::sqrt(D)) / 2*a;
		return {x1, x2};
	}
	else if (D < -EPSILON)
	{
		return {};
	}
	else
	{
		const auto x = -b / (2*a);
		return {x};
	}
}

TEST(MoveTestSuite, MoveObject)
{
	constexpr Point FinalPos {5, 8};

	MovableObject mobj(InitialPos, InitialVelocity);

	MoveCommand move(mobj);
	move.exec();

	ASSERT_EQ(mobj.getLocation(), FinalPos);
}

TEST(MoveTestSuite, MoveNoGetLocationObject)
{
	MovableButNotGetLocatable mobj;
	MoveCommand move(mobj);

	EXPECT_THROW(move.exec(), std::runtime_error);
}

TEST(MoveTestSuite, MoveNoVelocityObject)
{
	MovableButNotGetVelocitable mobj;
	MoveCommand move(mobj);

	EXPECT_THROW(move.exec(), std::runtime_error);
}

TEST(MoveTestSuite, MoveNoSetLocationObject)
{
	MovableButNotSetLocatable mobj;
	MoveCommand move(mobj);

	EXPECT_THROW(move.exec(), std::runtime_error);
}

TEST(RotateTestSuite, RotateObject)
{
	RotatableObject robj(10.0, 40.0);
	RotateCommand rotate(robj);

	rotate.exec();

	ASSERT_EQ(robj.getAngle(), 50.0);

	RotatableObject robj2(10.0, -40.0);
	RotateCommand rotate2(robj2);

	rotate2.exec();

	ASSERT_EQ(robj2.getAngle(), 330.0);

	RotatableObject robj3(350.0, 300.0);
	RotateCommand rotate3(robj3);

	rotate3.exec();

	ASSERT_EQ(robj3.getAngle(), 290.0);
}

// Items 3-4. x^2 + 1 = 0
TEST(SolveTestSuite, ABC101)
{
	const auto roots = solve(1, 0, 1);
	ASSERT_EQ(roots.size(), 0);
}

// Items 5-6. x^2 - 1 = 0
TEST(SolveTestSuite, ABC10m1)
{
	const auto roots = solve(1, 0, -1);
	ASSERT_EQ(roots.size(), 2);
	EXPECT_DOUBLE_EQ(roots.at(0), 1);
	EXPECT_DOUBLE_EQ(roots.at(1), -1);
}

// Items 7-8. x^2 + 2*x + 1 = 0
TEST(SolveTestSuite, ABC121)
{
	const auto roots = solve(1, 2, 1);
	ASSERT_EQ(roots.size(), 1);
	EXPECT_DOUBLE_EQ(roots.at(0), -1);
}

// Items 9-10. a = 0
TEST(SolveTestSuite, ZeroA)
{
	EXPECT_THROW(solve(0, 3.1, 2.3), std::invalid_argument);
}

// Items 11-12. x^2 + (2+1e-10)*x + 1+1e-10 = 0
TEST(SolveTestSuite, ABC121em10)
{
	const auto a = 1.0;
	const auto b = 2.0;
	const auto c = 1.0 - (EPSILON/10);
	const auto roots = solve(a, b, c);
	ASSERT_EQ(roots.size(), 1);
	EXPECT_NEAR(roots.at(0), -1, EPSILON);

	const auto D = b*b - 4*a*c;
	EXPECT_LT(D, EPSILON);
	EXPECT_GT(D, 0.0);
}

// Items 13-14. NaN or Infinite coefficients
TEST(SolveTestSuite, NotFiniteABC)
{
	EXPECT_THROW(solve(NAN, 2.2, 10.0), std::invalid_argument);
	EXPECT_THROW(solve(1.0, INFINITY, 10.0), std::invalid_argument);
	EXPECT_THROW(solve(1.0, 2.2, NAN), std::invalid_argument);
}

#include "BurnFuelCommand.h"
#include "ChangeVelocityCommand.h"
#include "CheckFuelCommand.h"
#include "FuelableObject.h"
#include "MacroCommand.h"
#include "RotateAndChangeVelocityCommand.h"

const FuelUnit InitialFuel = 12.0;
const FuelUnit InitalConsumption = 5.0;

TEST(CommandTestSuite, CheckEnoughFuel)
{
	FuelableObject fObj(InitialFuel, InitalConsumption);

	CheckFuelCommand checkFuelCommand(fObj);
	EXPECT_NO_THROW(checkFuelCommand.exec());
}

TEST(CommandTestSuite, CheckNotEnoughFuel)
{
	FuelableObject fObj(2.0, 3.0);

	CheckFuelCommand checkFuelCommand(fObj);
	EXPECT_THROW(checkFuelCommand.exec(), CommandException);
}

TEST(CommandTestSuite, BurnFuel)
{
	FuelableObject fObj(InitialFuel, InitalConsumption);

	BurnFuelCommand burnFuelCommand(fObj);
	burnFuelCommand.exec();
	EXPECT_DOUBLE_EQ(fObj.getFuelLevel(), 7.0);

	burnFuelCommand.exec();
	EXPECT_DOUBLE_EQ(fObj.getFuelLevel(), 2.0);
}

TEST(CommandTestSuite, MacroCommandEnoughFuel)
{
	FuelableObject fObj(InitialFuel, InitalConsumption);
	MovableObject mObj(InitialPos, InitialVelocity);

	std::vector<ICommandUPtr> commands;
	commands.push_back(std::make_unique<CheckFuelCommand>(fObj));
	commands.push_back(std::make_unique<MoveCommand>(mObj));
	commands.push_back(std::make_unique<BurnFuelCommand>(fObj));

	MacroCommand macroCommand(std::move(commands));

	EXPECT_NO_THROW(macroCommand.exec());
	EXPECT_DOUBLE_EQ(fObj.getFuelLevel(), 7.0);
	EXPECT_EQ(mObj.getLocation(), Point(5.0, 8.0));
}

TEST(CommandTestSuite, MacroCommandNotEnoughFuel)
{
	FuelableObject fObj(2.0, InitalConsumption);
	MovableObject mObj(InitialPos, InitialVelocity);

	std::vector<ICommandUPtr> commands;
	commands.push_back(std::make_unique<CheckFuelCommand>(fObj));
	commands.push_back(std::make_unique<MoveCommand>(mObj));
	commands.push_back(std::make_unique<BurnFuelCommand>(fObj));

	MacroCommand macroCommand(std::move(commands));

	EXPECT_THROW(macroCommand.exec(), CommandException);
	EXPECT_DOUBLE_EQ(fObj.getFuelLevel(), 2.0);
	EXPECT_EQ(mObj.getLocation(), InitialPos);
}

TEST(CommandTestSuite, ChangeVelocityCommandForUnmovableObject)
{
	RotatableObject rObj(0, 90);

	ChangeVelocityCommand changeVelocityCommand(&rObj, nullptr);

	EXPECT_NO_THROW(changeVelocityCommand.exec());
}

TEST(CommandTestSuite, ChangeVelocityCommandForMovableObject)
{
	RotatableObject rObj(0, 90);
	MovableObject mObj(InitialPos, {3, 0});

	ChangeVelocityCommand changeVelocityCommand(&rObj, &mObj);
	changeVelocityCommand.exec();

	EXPECT_EQ(mObj.getVelocity(), Point(0, 3));
}

TEST(CommandTestSuite, RotateAndChangeVelocityForUnmovableObject)
{
	RotatableObject rObj(0, 90);

	RotateAndChangeVelocityCommand rotateAndChangeVelocityCommand(&rObj, nullptr);

	EXPECT_NO_THROW(rotateAndChangeVelocityCommand.exec());

	EXPECT_EQ(rObj.getAngle(), 90);
}

TEST(CommandTestSuite, RotateAndChangeVelocityForMovableObject)
{
	RotatableObject rObj(0, 90);
	MovableObject mObj(InitialPos, {3, 0});

	RotateAndChangeVelocityCommand rotateAndChangeVelocityCommand(&rObj, &mObj);
	rotateAndChangeVelocityCommand.exec();

	EXPECT_EQ(rObj.getAngle(), 90);
	EXPECT_EQ(mObj.getVelocity(), Point(0, 3));
}

int main(int argc, char** argv)
{
	ExceptionHandler::registrate(typeid(LogCommand), typeid(std::exception), logHandler);
	ExceptionHandler::registrate(typeid(RetryCommand), typeid(std::exception), retryHandler);

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
