#pragma once

#include "ICommand.h"
#include "ThreadSafeQueue.h"

#include <thread>

using ThreadSafeQueueCommandPtr = std::shared_ptr<ThreadSafeQueue<ICommandPtr>>;

class ServerThread
{
public:
    ServerThread(ThreadSafeQueueCommandPtr queue);
    
    void start();
    void stop();
    void softStop();

    void join();
    
    bool isRunning() const;

private:
    void run();

private:
    ThreadSafeQueueCommandPtr _queue;
    std::unique_ptr<std::thread> _thread;
    bool _isRunning = false;
    bool _isSoftStopActivated = false;
};
