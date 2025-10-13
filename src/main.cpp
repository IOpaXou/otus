#include "CommandException.h"
#include "ExceptionHandler.h"
#include "DoubleRetryCommand.h"
#include "FailedCommand.h"
#include "LogCommand.h"
#include "RetryCommand.h"

#include "IMovable.h"
#include "MoveCommand.h"
#include "MovableObject.h"
#include "RotateCommand.h"
#include "RotatableObject.h"

#include "BurnFuelCommand.h"
#include "ChangeVelocityCommand.h"
#include "CheckFuelCommand.h"
#include "FuelableObject.h"
#include "MacroCommand.h"
#include "RotateAndChangeVelocityCommand.h"

#include "IoCHelper.h"

#include "FinishCommand.h"
#include "SetLocationCommand.h"
#include "TestMovableAdapter.h"
#include "UObject.h"

#include "HardStopCommand.h"
#include "ServerThread.h"
#include "SoftStopCommand.h"
#include "StartCommand.h"
#include "TestCommand.h"

#include "GameMessage.h"
#include "GameQueueRepositoryImpl.h"
#include "HttpEndpoint.h"
#include "InterpretCommand.h"
#include "IGameQueueRepository.h"
#include "IQueueImpl.h"
#include "TestDependencies.h"

#include "AuthService.h"
#include "JWTUtils.h"

#include "json.hpp"

#include <chrono>
#include <climits>
#include <gtest/gtest.h>
#include <memory>

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
	void finish() override {}
};

using MovableButNotGetLocatablePtr = std::shared_ptr<MovableButNotGetLocatable>;

class MovableButNotGetVelocitable : public IMovable
{
public:
	Point getLocation() override {return {};}
	Vector getVelocity() override {
		throw std::runtime_error("Can't get velocity");
	}
	void setLocation(const Point&) override {}
	void setVelocity(const Vector&) override {}
	void finish() override {}
};

using MovableButNotGetVelocitablePtr = std::shared_ptr<MovableButNotGetVelocitable>;

class MovableButNotSetLocatable : public IMovable
{
public:
	Point getLocation() override {return {};}
	Vector getVelocity() override {return {};}
	void setLocation(const Point&) override {
		throw std::runtime_error("Can't set location");
	}
	void setVelocity(const Vector&) override {}
	void finish() override {}
};

using MovableButNotSetLocatablePtr = std::shared_ptr<MovableButNotSetLocatable>;

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

	MovableObjectPtr mobj = std::make_shared<MovableObject>(InitialPos, InitialVelocity);

	MoveCommand move(mobj);
	move.exec();

	ASSERT_EQ(mobj->getLocation(), FinalPos);
}

TEST(MoveTestSuite, MoveNoGetLocationObject)
{
	MovableButNotGetLocatablePtr mobj = std::make_shared<MovableButNotGetLocatable>();
	MoveCommand move(mobj);

	EXPECT_THROW(move.exec(), std::runtime_error);
}

TEST(MoveTestSuite, MoveNoVelocityObject)
{
	MovableButNotGetVelocitablePtr mobj = std::make_shared<MovableButNotGetVelocitable>();
	MoveCommand move(mobj);

	EXPECT_THROW(move.exec(), std::runtime_error);
}

TEST(MoveTestSuite, MoveNoSetLocationObject)
{
	MovableButNotSetLocatablePtr mobj = std::make_shared<MovableButNotSetLocatable>();
	MoveCommand move(mobj);

	EXPECT_THROW(move.exec(), std::runtime_error);
}

TEST(RotateTestSuite, RotateObject)
{
	RotatableObjectPtr robj = std::make_shared<RotatableObject>(10.0, 40.0);
	RotateCommand rotate(robj);

	rotate.exec();

	ASSERT_EQ(robj->getAngle(), 50.0);

	RotatableObjectPtr robj2 = std::make_shared<RotatableObject>(10.0, -40.0);
	RotateCommand rotate2(robj2);

	rotate2.exec();

	ASSERT_EQ(robj2->getAngle(), 330.0);

	RotatableObjectPtr robj3 = std::make_shared<RotatableObject>(350.0, 300.0);
	RotateCommand rotate3(robj3);

	rotate3.exec();

	ASSERT_EQ(robj3->getAngle(), 290.0);
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
	EXPECT_EQ(roots.size(), 1);
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

const FuelUnit InitialFuel = 12.0;
const FuelUnit InitalConsumption = 5.0;

TEST(CommandTestSuite, CheckEnoughFuel)
{
	FuelableObjectPtr fObj = std::make_shared<FuelableObject>(InitialFuel, InitalConsumption);

	CheckFuelCommand checkFuelCommand(fObj);
	EXPECT_NO_THROW(checkFuelCommand.exec());
}

TEST(CommandTestSuite, CheckNotEnoughFuel)
{
	FuelableObjectPtr fObj = std::make_shared<FuelableObject>(2.0, 3.0);

	CheckFuelCommand checkFuelCommand(fObj);
	EXPECT_THROW(checkFuelCommand.exec(), CommandException);
}

TEST(CommandTestSuite, BurnFuel)
{
	FuelableObjectPtr fObj = std::make_shared<FuelableObject>(InitialFuel, InitalConsumption);

	BurnFuelCommand burnFuelCommand(fObj);
	burnFuelCommand.exec();
	EXPECT_DOUBLE_EQ(fObj->getFuelLevel(), 7.0);

	burnFuelCommand.exec();
	EXPECT_DOUBLE_EQ(fObj->getFuelLevel(), 2.0);
}

TEST(CommandTestSuite, MacroCommandEnoughFuel)
{
	FuelableObjectPtr fObj = std::make_shared<FuelableObject>(InitialFuel, InitalConsumption);
	MovableObjectPtr mObj = std::make_shared<MovableObject>(InitialPos, InitialVelocity);

	std::vector<ICommandUPtr> commands;
	commands.push_back(std::make_unique<CheckFuelCommand>(fObj));
	commands.push_back(std::make_unique<MoveCommand>(mObj));
	commands.push_back(std::make_unique<BurnFuelCommand>(fObj));

	MacroCommand macroCommand(std::move(commands));

	EXPECT_NO_THROW(macroCommand.exec());
	EXPECT_DOUBLE_EQ(fObj->getFuelLevel(), 7.0);
	EXPECT_EQ(mObj->getLocation(), Point(5.0, 8.0));
}

TEST(CommandTestSuite, MacroCommandNotEnoughFuel)
{
	FuelableObjectPtr fObj = std::make_shared<FuelableObject>(2.0, InitalConsumption);
	MovableObjectPtr mObj = std::make_shared<MovableObject>(InitialPos, InitialVelocity);

	std::vector<ICommandUPtr> commands;
	commands.push_back(std::make_unique<CheckFuelCommand>(fObj));
	commands.push_back(std::make_unique<MoveCommand>(mObj));
	commands.push_back(std::make_unique<BurnFuelCommand>(fObj));

	MacroCommand macroCommand(std::move(commands));

	EXPECT_THROW(macroCommand.exec(), CommandException);
	EXPECT_DOUBLE_EQ(fObj->getFuelLevel(), 2.0);
	EXPECT_EQ(mObj->getLocation(), InitialPos);
}

TEST(CommandTestSuite, ChangeVelocityCommandForUnmovableObject)
{
	RotatableObjectPtr rObj = std::make_shared<RotatableObject>(0, 90);

	ChangeVelocityCommand changeVelocityCommand(rObj, nullptr);

	EXPECT_NO_THROW(changeVelocityCommand.exec());
}

TEST(CommandTestSuite, ChangeVelocityCommandForMovableObject)
{
	RotatableObjectPtr rObj = std::make_shared<RotatableObject>(0, 90);
	MovableObjectPtr mObj = std::make_shared<MovableObject>(InitialPos, Vector{3, 0});

	ChangeVelocityCommand changeVelocityCommand(rObj, mObj);
	changeVelocityCommand.exec();

	EXPECT_EQ(mObj->getVelocity(), Point(0, 3));
}

TEST(CommandTestSuite, RotateAndChangeVelocityForUnmovableObject)
{
	RotatableObjectPtr rObj = std::make_shared<RotatableObject>(0, 90);

	RotateAndChangeVelocityCommand rotateAndChangeVelocityCommand(rObj, nullptr);

	EXPECT_NO_THROW(rotateAndChangeVelocityCommand.exec());

	EXPECT_EQ(rObj->getAngle(), 90);
}

TEST(CommandTestSuite, RotateAndChangeVelocityForMovableObject)
{
	RotatableObjectPtr rObj = std::make_shared<RotatableObject>(0, 90);
	MovableObjectPtr mObj = std::make_shared<MovableObject>(InitialPos, Vector{3, 0});

	RotateAndChangeVelocityCommand rotateAndChangeVelocityCommand(rObj, mObj);
	rotateAndChangeVelocityCommand.exec();

	EXPECT_EQ(rObj->getAngle(), 90);
	EXPECT_EQ(mObj->getVelocity(), Point(0, 3));
}

class IoCSample
{
public:
	IoCSample(int val, const std::string& str) : _val(val), _str(str) {}
	int value() const {
		return _val;
	}
	std::string str()
	{
		return _str;
	}
private:
	int _val{};
	std::string _str{};
};

TEST(IoCTestSuite, IoCResolve)
{
	auto dependencyName = "IoCSample";
	Factory dependencyFactory = [](const std::vector<AnyValue>& args){
		if (args.empty()) {
			throw std::runtime_error("IoC Sample requires 1 argument");
		}
		return std::make_shared<IoCSample>(std::any_cast<int>(args[0]), std::any_cast<std::string>(args[1]));
	};

	EXPECT_NO_THROW(registerFactoryHelper(dependencyName, dependencyFactory)->exec());

	const auto iocSampleVal = 2;
	const std::string iocSampleStr = "Strochka";
	const auto iocSample = IoC::Resolve<std::shared_ptr<IoCSample>>(dependencyName, {iocSampleVal, iocSampleStr});

	EXPECT_TRUE(iocSample);
	EXPECT_EQ(iocSample->value(), 2);
	EXPECT_EQ(iocSample->str(), iocSampleStr);

	dependencyName = "IntResolve";
	dependencyFactory = [](const std::vector<AnyValue>& args) {
		return 99;
	};
	EXPECT_NO_THROW(registerFactoryHelper(dependencyName, dependencyFactory)->exec());
	auto iocIntResolve = IoC::Resolve<int>(dependencyName);
	EXPECT_EQ(iocIntResolve, 99);

	dependencyName = "StringResolve";
	dependencyFactory = [](const std::vector<AnyValue>& args) {
		return std::string("66");
	};
	EXPECT_NO_THROW(registerFactoryHelper(dependencyName, dependencyFactory)->exec());
	std::string iocStringResolve = IoC::Resolve<std::string>(dependencyName);
	EXPECT_EQ(iocStringResolve, "66");
}

TEST(IoCTestSuite, IoCResolveThrow)
{
	EXPECT_THROW(IoC::Resolve<void>("NotExistedDependency"), std::runtime_error);
	EXPECT_THROW(IoC::Resolve<void>("IoC.Register", {"DependencyName"}), std::invalid_argument);
}

TEST(IoCTestSuite, ScopesManagement)
{
	std::string newScope = "newScope";

	auto created = IoC::Resolve<bool>("Scopes.New.Impl", {newScope});
	EXPECT_TRUE(created);
	created = IoC::Resolve<bool>("Scopes.New.Impl", {newScope});
	EXPECT_FALSE(created);

	auto current = IoC::Resolve<bool>("Scopes.Current.Impl", {std::string("smth")});
	EXPECT_FALSE(current);
	current = IoC::Resolve<bool>("Scopes.Current.Impl", {newScope});
	EXPECT_TRUE(current);

	auto cleaned = IoC::Resolve<bool>("Scopes.Clear.Impl", {newScope});
	EXPECT_TRUE(cleaned);

	current = IoC::Resolve<bool>("Scopes.Current.Impl", {newScope});
	EXPECT_FALSE(current);
}

TEST(IoCTestSuite, ScopesIsolation)
{
	std::string scope1 = "scope1";
	std::string scope2 = "scope2";
	std::string dependencyName = "dependency";

	IoC::Resolve<ICommandPtr>("Scopes.New", {scope1})->exec();
	IoC::Resolve<ICommandPtr>("Scopes.New", {scope2})->exec();

	IoC::Resolve<ICommandPtr>("Scopes.Current", {scope1})->exec();
	registerFactoryHelper(dependencyName, [](const auto&) { return 1;})->exec();

	IoC::Resolve<ICommandPtr>("Scopes.Current", {scope2})->exec();
	registerFactoryHelper(dependencyName, [](const auto&) { return 2;})->exec();

	IoC::Resolve<ICommandPtr>("Scopes.Current", {scope1})->exec();
	EXPECT_EQ(IoC::Resolve<int>(dependencyName), 1);
	IoC::Resolve<ICommandPtr>("Scopes.Current", {scope2})->exec();
	EXPECT_EQ(IoC::Resolve<int>(dependencyName), 2);
}

TEST(IoCTestSuite, Multithreading)
{
	constexpr int threadsCount = 5;
	std::vector<std::thread> threads;
	std::atomic<int> done = 0;

	std::string multithreadDependency = "multithreadDependency";

	for (auto t = 0; t < threadsCount; t++)
	{
		threads.emplace_back([t, &multithreadDependency, &done](){
			std::string scopeName = "Scope" + std::to_string(t);
			IoC::Resolve<ICommandPtr>("Scopes.New", {scopeName})->exec();
			IoC::Resolve<ICommandPtr>("Scopes.Current", {scopeName})->exec();

			registerFactoryHelper(multithreadDependency, [t](const auto&){
				return t*100;
			})->exec();

			EXPECT_EQ(IoC::Resolve<int>(multithreadDependency), t*100);
			done++;
		});
	}

	for (auto& thread : threads)
	{
		thread.join();
	}
	EXPECT_EQ(done, threadsCount);
}

TEST(AdapterTestSuite, TestAdapterGenerator)
{
	RegisterIMovableTestMovableAdapter();

	UObjectPtr spaceship = std::make_shared<UObject>();

	spaceship->setProperty<Point>(UObject::LocationProperty, Point(100, 200));
	spaceship->setProperty<Vector>(UObject::VelocityProperty, Vector(300, 400));

	auto getLocationRegisterCommand = registerFactoryHelper("IMovable:getLocation", [](const std::vector<AnyValue>& args) {
		auto obj = std::any_cast<UObjectPtr>(args[0]);
        return obj->getProperty<Point>(UObject::LocationProperty);
    });
	EXPECT_TRUE(getLocationRegisterCommand);
	getLocationRegisterCommand->exec();

	auto setLocationRegisterCommand = registerFactoryHelper("IMovable:setLocation", [](const std::vector<AnyValue>& args) {
		auto command = std::make_shared<SetLocationCommand>(args);
		return std::static_pointer_cast<ICommand>(command);
    });
	EXPECT_TRUE(setLocationRegisterCommand);
	setLocationRegisterCommand->exec();

	auto getVelocityRegisterCommand = registerFactoryHelper("IMovable:getVelocity", [](const std::vector<AnyValue>& args) {
		auto obj = std::any_cast<UObjectPtr>(args[0]);
        return obj->getProperty<Vector>(UObject::VelocityProperty);
    });
	EXPECT_TRUE(getVelocityRegisterCommand);
	getVelocityRegisterCommand->exec();

	auto finishCommand = registerFactoryHelper("IMovable:finish", [](const std::vector<AnyValue>& args) {
		auto command = std::make_shared<FinishCommand>(args);
		return std::static_pointer_cast<ICommand>(command);
	});
	EXPECT_TRUE(finishCommand);
	finishCommand->exec();

	auto adapter = IoC::Resolve<IMovablePtr>("IMovable.Adapter", {spaceship});
	auto adapter2 = IoC::Resolve<IMovablePtr>("IMovable.Adapter", {spaceship});
	EXPECT_NE(adapter, nullptr);
	EXPECT_NE(adapter2, nullptr);
	EXPECT_NE(adapter, adapter2);

	EXPECT_EQ(adapter->getLocation(), Point(100, 200));
	EXPECT_EQ(adapter->getVelocity(), Vector(300, 400));

	EXPECT_NO_THROW(adapter->setLocation({500, 600}));
	EXPECT_EQ(adapter->getLocation(), Point(500, 600));
	EXPECT_EQ(adapter2->getLocation(), Point(500, 600));

	EXPECT_THROW(spaceship->getProperty<bool>(UObject::FinishProperty), std::runtime_error);
	adapter->finish();
	EXPECT_EQ(spaceship->getProperty<bool>(UObject::FinishProperty), true);
}

TEST(MultithreadingTestSuite, TestStartCommand)
{
	ThreadSafeQueueCommandPtr queue = std::make_shared<ThreadSafeQueue<ICommandPtr>>();
	ServerThread serverThread(queue);
	StartCommand startCmd(serverThread);

	EXPECT_FALSE(serverThread.isRunning());
	startCmd.exec();
	EXPECT_TRUE(serverThread.isRunning());

	serverThread.stop();
	serverThread.join();
}

TEST(MultithreadingTestSuite, TestConditionalStartCommand)
{
	ThreadSafeQueueCommandPtr queue = std::make_shared<ThreadSafeQueue<ICommandPtr>>();
	ServerThread serverThread(queue);
	StartCommand startCmd(serverThread);

	std::thread::id serverThreadID;
	std::mutex mutex;
	std::condition_variable cv;

	EXPECT_FALSE(serverThread.isRunning());
	queue->push(std::make_shared<TestCommand>([&]() {
		{
			serverThreadID = std::this_thread::get_id();
			std::lock_guard<std::mutex> lock(mutex);
			cv.notify_one();
		}
	}));

	startCmd.exec();
	{
		std::unique_lock<std::mutex> lock(mutex);
		cv.wait_for(lock, std::chrono::seconds(2), [&serverThread](){
			return serverThread.isRunning();
		});
	}

	EXPECT_NE(serverThreadID, std::this_thread::get_id());
	EXPECT_TRUE(serverThread.isRunning());

	serverThread.stop();
	serverThread.join();
}

TEST(MultithreadingTestSuite, TestHardStopCommand)
{
	ThreadSafeQueueCommandPtr queue = std::make_shared<ThreadSafeQueue<ICommandPtr>>();
	ServerThread serverThread(queue);
	StartCommand startCmd(serverThread);

	bool hardStop = true;
	bool softStop = false;
	std::mutex mutex;
	std::condition_variable cv;

	queue->push(std::make_shared<TestCommand>([](){}));
	queue->push(std::make_shared<TestCommand>([](){}));
	queue->push(std::make_shared<TestCommand>([](){}));
	queue->push(std::make_shared<HardStopCommand>(serverThread));
	queue->push(std::make_shared<TestCommand>([&](){
		std::lock_guard<std::mutex> lock(mutex);
		hardStop = false;
		cv.notify_one();
	}));
	queue->push(std::make_shared<TestCommand>([](){}));
	queue->push(std::make_shared<TestCommand>([&](){
		softStop = true;
	}));

	EXPECT_FALSE(serverThread.isRunning());
	startCmd.exec();
	EXPECT_TRUE(serverThread.isRunning());

	std::unique_lock<std::mutex> lock(mutex);
	cv.wait_for(lock, std::chrono::milliseconds(100),[&hardStop](){ return !hardStop;});

	EXPECT_TRUE(hardStop);
	EXPECT_FALSE(softStop);
	EXPECT_FALSE(queue->empty());

	serverThread.join();
}

TEST(MultithreadingTestSuite, TestSoftStopCommand)
{
	ThreadSafeQueueCommandPtr queue = std::make_shared<ThreadSafeQueue<ICommandPtr>>();
	ServerThread serverThread(queue);
	StartCommand startCmd(serverThread);

	bool hardStop = true;
	bool softStop = false;
	std::mutex mutex;
	std::condition_variable cv;

	queue->push(std::make_shared<TestCommand>([](){}));
	queue->push(std::make_shared<TestCommand>([](){}));
	queue->push(std::make_shared<TestCommand>([](){}));
	queue->push(std::make_shared<SoftStopCommand>(serverThread));
	queue->push(std::make_shared<TestCommand>([&](){
		std::lock_guard<std::mutex> lock(mutex);
		hardStop = false;
		cv.notify_one();
	}));
	queue->push(std::make_shared<TestCommand>([](){}));
	queue->push(std::make_shared<TestCommand>([&](){
		std::lock_guard<std::mutex> lock(mutex);
		softStop = true;
		cv.notify_one();
	}));

	EXPECT_FALSE(serverThread.isRunning());
	startCmd.exec();
	EXPECT_TRUE(serverThread.isRunning());

	std::unique_lock<std::mutex> lock(mutex);
	cv.wait_for(lock, std::chrono::milliseconds(100),[&hardStop](){ return !hardStop;});
	EXPECT_FALSE(hardStop);

	cv.wait_for(lock, std::chrono::milliseconds(100), [&softStop](){ return softStop;});
	EXPECT_TRUE(softStop);
	EXPECT_TRUE(queue->empty());

	serverThread.join();
}

TEST(EndpointTestSuite, TestParseJSON)
{
	std::string jsonMsg = R"({
            "gameId": "game_123",
            "objectId": "ship_456",
            "commandId": "Move",
            "args": ["north", 100, 5.5, true]
        })";

	EXPECT_NO_THROW({
		auto message = GameMessage::fromJSON(jsonMsg);

	EXPECT_EQ(message.gameId, "game_123");
	EXPECT_EQ(message.objectId, "ship_456");
	EXPECT_EQ(message.commandId, "Move");
	EXPECT_EQ(message.args.size(), 4);
	EXPECT_EQ(std::any_cast<std::string>(message.args[0]), "north");
	EXPECT_EQ(std::any_cast<int>(message.args[1]), 100);
	EXPECT_DOUBLE_EQ(std::any_cast<double>(message.args[2]), 5.5);
	EXPECT_EQ(std::any_cast<bool>(message.args[3]), true);
    });
}

const std::string gameId = "game_123";
const std::string objectId = "ship_456";

TEST(EndpointTestSuite, TestInterpretCommand)
{
	registerHttpEndpointTestDependecies();

	auto testObject = std::make_shared<UObject>();
	registerFactoryHelper("GameObjects.Get", [testObject](const std::vector<AnyValue>& args) {
        return testObject;
    })->exec();

	EXPECT_FALSE(testObject->hasProperty(UObject::LocationProperty));
	EXPECT_FALSE(testObject->hasProperty(UObject::VelocityProperty));

	GameMessage message {gameId, objectId, "Move", {2.0, 3.0, 4.0, 5.0}};
	IQueuePtr<ICommandPtr> queue = std::make_shared<IQueueImpl<ICommandPtr>>();
	auto interpretCommand = std::make_shared<InterpretCommand>(message, queue);

	EXPECT_TRUE(queue->empty());
	ASSERT_NE(interpretCommand, nullptr);
	EXPECT_NO_THROW(interpretCommand->exec());
	EXPECT_FALSE(queue->empty());

	EXPECT_TRUE(testObject->hasProperty(UObject::LocationProperty));
	EXPECT_TRUE(testObject->hasProperty(UObject::VelocityProperty));

	auto location = testObject->getProperty<Point>(UObject::LocationProperty);
	auto velocity = testObject->getProperty<Vector>(UObject::VelocityProperty);

	EXPECT_DOUBLE_EQ(location.first, 2.0);
	EXPECT_DOUBLE_EQ(location.second, 3.0);
	EXPECT_DOUBLE_EQ(velocity.first, 4.0);
	EXPECT_DOUBLE_EQ(velocity.second, 5.0);

	ICommandPtr cmd = queue->pop();
	ASSERT_NE(cmd, nullptr);

	auto moveCommandPtr = std::dynamic_pointer_cast<MoveCommand>(cmd);
	EXPECT_NE(moveCommandPtr, nullptr);
}

TEST(EndpointTestSuite, TestInterpretCommandThrow)
{
	registerHttpEndpointTestDependecies();
	GameMessage message {gameId, objectId, "Move", {2.0, 3.0, 4.0}};

	IQueuePtr<ICommandPtr> queue = std::make_shared<IQueueImpl<ICommandPtr>>();
	auto interpretCommand = std::make_shared<InterpretCommand>(message, queue);

	EXPECT_THROW(interpretCommand->exec(), std::runtime_error);
}

TEST(EndpointTestSuite, TestHttpEndpoint)
{
	const std::string testHost = "localhost";
	const auto testPort = 8082;

	registerHttpEndpointTestDependecies();

	IQueuePtr<ICommandPtr> queue = std::make_shared<IQueueImpl<ICommandPtr>>();
	std::unordered_map<std::string, IQueuePtr<ICommandPtr>> queueMap {{gameId, queue}};

	IGameQueueRepositoryPtr gameQueueRepository = std::make_shared<GameQueueRepositoryImpl>(queueMap);

	HttpEndpoint httpEndpoint(testHost, testPort, gameQueueRepository);

	std::thread endpointThread([&httpEndpoint](){
		httpEndpoint.start();
	});

	// Time to launch httpEndpoint
	std::this_thread::sleep_for(std::chrono::milliseconds(50));

	EXPECT_TRUE(queue->empty());

	httplib::Client client(testHost, testPort);
	auto response = client.Post("/command",
        R"({"gameId": "game_123", "objectId": "ship_456", "commandId": "Move", "args": [1,2,3,4]})",
        "application/json");

	ASSERT_TRUE(response);
	EXPECT_EQ(response->status, 200);

	EXPECT_FALSE(queue->empty());

	EXPECT_TRUE(httpEndpoint.isRunning());
	httpEndpoint.stop();
	EXPECT_FALSE(httpEndpoint.isRunning());

	ICommandPtr cmd = queue->pop();
	ASSERT_NE(cmd, nullptr);

	auto interpretCommandPtr = std::dynamic_pointer_cast<InterpretCommand>(cmd);
	EXPECT_NE(interpretCommandPtr, nullptr);

	endpointThread.join();
}

TEST(AuthorizationTestSuite, TestJWT)
{
    std::string privateKey, publicKey;
    JWTUtils::generateRSAKeys(privateKey, publicKey);

    std::string userId = "test";
    std::string gameId = "game_1_1234567890";

    std::string token = JWTUtils::generateToken(userId, gameId, privateKey);
    bool isValid = JWTUtils::verifyToken(token, publicKey);
    EXPECT_TRUE(isValid);

    std::string invalidToken = token.substr(0, token.length() - 5) + "invalid";
    bool isInvalid = JWTUtils::verifyToken(invalidToken, publicKey);
    EXPECT_FALSE(isInvalid);

    std::string wrongPrivateKey, wrongPublicKey;
    JWTUtils::generateRSAKeys(wrongPrivateKey, wrongPublicKey);

    bool isWrongKey = JWTUtils::verifyToken(token, wrongPublicKey);
    EXPECT_FALSE(isWrongKey);
}

TEST(AuthorizationTestSuite, TestCreateGame)
{
	AuthService authService;

	const std::string owner = "owner";
	const std::vector<std::string> participants {"part1", "part2"};
	auto newGameId = authService.createGame(owner, participants);
	EXPECT_TRUE(!newGameId.empty());
	EXPECT_EQ(newGameId.find("game_"), 0);
}

TEST(AuthorizationTestSuite, TestIssueToken)
{
	AuthService authService;

	const std::string owner = "owner";
	const std::vector<std::string> participants {"part1", "part2"};

	auto newGameId = authService.createGame(owner, participants);

	std::string token;
	EXPECT_NO_THROW(token = authService.issueToken(newGameId, "part1"));
	EXPECT_TRUE(!token.empty());
	EXPECT_THROW(token = authService.issueToken(newGameId, "part3"), std::runtime_error);
	EXPECT_THROW(token = authService.issueToken("game_13123", "part1"), std::runtime_error);
}

TEST(AuthorizationTestSuite, TestJWTVerification)
{
	const std::string testHost = "localhost";
	const auto testPort = 8082;

	registerHttpEndpointTestDependecies();

	IQueuePtr<ICommandPtr> queue = std::make_shared<IQueueImpl<ICommandPtr>>();
	std::unordered_map<std::string, IQueuePtr<ICommandPtr>> queueMap;

	IGameQueueRepositoryPtr gameQueueRepository = std::make_shared<GameQueueRepositoryImpl>(queueMap);

	HttpEndpoint httpEndpoint(testHost, testPort, gameQueueRepository);
	httpEndpoint.setCheckAuth(true);

	std::thread endpointThread([&httpEndpoint](){
		httpEndpoint.start();
	});

	// Time to launch httpEndpoint
	std::this_thread::sleep_for(std::chrono::milliseconds(50));

	EXPECT_TRUE(queue->empty());

	httplib::Client client(testHost, testPort);
	auto response = client.Post("/auth/game/create",
		R"({"owner": "owner", "participants": ["part1", "part2"]})","application/json");
	EXPECT_EQ(response->status, 201);

	auto json = nlohmann::json::parse(response->body);
	std::string gameId = json["gameId"];
	auto createdGame = gameQueueRepository->findGameQueueById(gameId);
	EXPECT_TRUE(createdGame);

	std::string authRequest = R"({"userId": "part2", "gameId": ")" + gameId + R"("})";
	response = client.Post("/auth/token",authRequest,"application/json");
	EXPECT_EQ(response->status, 200);

	json = nlohmann::json::parse(response->body);
	std::string token = json["token"];
	EXPECT_TRUE(!token.empty());

	response = client.Post("/command",
	R"({"gameId": "game_123", "objectId": "ship_456", "commandId": "Move", "args": [1,2,3,4]})",
	"application/json");
	EXPECT_EQ(response->status, 403);

	std::string commandRequest =  R"({"gameId": ")" + gameId +
								  R"(", "jwt": ")" + token +
								  R"(", "objectId": "ship_456", "commandId": "Move", "args": [1,2,3,4]})";
	response = client.Post("/command", commandRequest, "application/json");
	EXPECT_EQ(response->status, 200);

	httpEndpoint.stop();
	endpointThread.join();
}

int main(int argc, char** argv)
{
	ExceptionHandler::registrate(typeid(LogCommand), typeid(std::exception), logHandler);
	ExceptionHandler::registrate(typeid(RetryCommand), typeid(std::exception), retryHandler);

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
