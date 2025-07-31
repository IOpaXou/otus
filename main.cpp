#include "CustomException.h"
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

int main(int argc, char** argv)
{
	ExceptionHandler::registrate(typeid(LogCommand), typeid(std::exception), logHandler);
	ExceptionHandler::registrate(typeid(RetryCommand), typeid(std::exception), retryHandler);

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

TEST(ExceptionTestSuite, LogCommand)
{
	CustomException customEx(customExText);
	ICommandUPtr logCmd = std::make_unique<LogCommand>(customEx);
	logCmd->exec();

	checkLog(customExText);
}

TEST(ExceptionTestSuite, LogHandler)
{
	const std::string failedCmdText = "FailedLogHandlerTest";

	CommandQueuePtr cmdQueue = std::make_shared<CommandQueue>();

	CustomException customEx(customExText);
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
	ExceptionHandler::registrate(typeid(FailedCommand), typeid(CustomException), retryHandler);
	ExceptionHandler::registrate(typeid(RetryCommand), typeid(CustomException), doubleRetryHandler);
	ExceptionHandler::registrate(typeid(DoubleRetryCommand), typeid(CustomException), logHandler);

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
