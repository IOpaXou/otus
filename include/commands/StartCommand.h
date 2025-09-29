#pragma once

#include "ICommand.h"
#include "ServerThread.h"

class StartCommand : public ICommand
{
public:
    StartCommand(ServerThread& serverThread) : _serverThread(serverThread) {}

    void exec() override
    {
        _serverThread.start();
    }

private:
    ServerThread& _serverThread;
};
