#pragma once

#include <exception>
#include <string>

class CustomException : public std::exception
{
public:
    CustomException(const std::string& msg) : _msg(msg) {}

    const char* what() const noexcept override
    {
        return _msg.c_str();
    }

private:
    std::string _msg;
};