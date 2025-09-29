#pragma once

#include "ICommand.h"
#include "ServerThread.h"

class SoftStopCommand : public ICommand
{
public:
    SoftStopCommand(ServerThread& serverThread) : _serverThread(serverThread) {}

    void exec() override
    {
        _serverThread.softStop();
    }

private:
    ServerThread& _serverThread;
};
