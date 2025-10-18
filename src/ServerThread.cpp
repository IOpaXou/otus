#include "ServerThread.h"

ServerThread::ServerThread(ThreadSafeQueueCommandPtr queue, IStatePtr state) : _queue(queue), _state(state)
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

IStatePtr ServerThread::state() const
{
    return _state;
}

void ServerThread::run()
{
    while (_isRunning)
    {
        auto cmd = _queue->pop();
        
        try
        {
            auto newState = _state->handle(cmd);
            if (!newState)
            {
                stop();
            }
            else
            {
                _state = newState;
            }
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
