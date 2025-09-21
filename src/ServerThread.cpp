#include "ServerThread.h"

ServerThread::ServerThread(ThreadSafeQueueCommandPtr queue) : _queue(queue)
{
}

void ServerThread::start()
{
    if (_isRunning)
    {
        return;
    }

    _isRunning = true;
    _thread = std::make_unique<std::thread>([this](){this->run();});
}

void ServerThread::stop()
{
    _isRunning = false;
    _isSoftStopActivated = false;
}

void ServerThread::softStop()
{
    _isRunning = false;
    _isSoftStopActivated = true;
}

void ServerThread::join()
{
    _thread->join();
}

bool ServerThread::isRunning() const
{
    return _isRunning;
}

void ServerThread::run()
{
    while (_isRunning)
    {
        auto cmd = _queue->pop();
        
        try
        {
            cmd->exec();
        }
        catch (...)
        {
        }
    }

    if (_isSoftStopActivated)
    {
        while (!_queue->empty())
        {
            auto cmd = _queue->pop();
            try
            {
                cmd->exec();
            }
            catch (...)
            {
            }
        }
    }
}
