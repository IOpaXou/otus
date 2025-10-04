#pragma once

#include "IQueue.h"

#include <queue>

template <typename T>
class IQueueImpl : public IQueue<T>
{
public:
    void push(T value)
    {
        _queue.push(value);
    }

    T pop()
    {
        T value = _queue.front();
        _queue.pop();
        return value;
    }

    bool empty() const
    {
        return _queue.empty();
    }
    
private:
    std::queue<T> _queue;
};
