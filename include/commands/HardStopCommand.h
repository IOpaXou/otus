#pragma once

#include "ICommand.h"
#include "ServerThread.h"

class HardStopCommand : public ICommand
{
public:
    HardStopCommand(ServerThread& serverThread) : _serverThread(serverThread) {}

    void exec() override
    {
        _serverThread.stop();
    }

private:
    ServerThread& _serverThread;
};
