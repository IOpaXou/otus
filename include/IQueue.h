#pragma once

#include <memory>

template <typename T>
class IQueue
{
public:
    virtual void push(T value) = 0;
    virtual T pop() = 0;
    virtual bool empty() const = 0;
};

template <typename T>
using IQueuePtr = std::shared_ptr<IQueue<T>>;
