#pragma once

#include <exception>
#include <string>

class CommandException : public std::exception
{
public:
    CommandException(const std::string& msg) : _msg(msg) {}

    const char* what() const noexcept override
    {
        return _msg.c_str();
    }

private:
    std::string _msg;
};
