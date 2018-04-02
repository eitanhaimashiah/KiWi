#pragma once

#include <stdexcept>

class NotImplementedException : public std::exception
{
private:
    std::string msg;

public:
    NotImplementedException(const std::string& message = "") : msg(message)
    {
    }

    const char * what() const throw()
    {
        return msg.c_str();
    }
};

class UnsupportedOperationException : public std::exception
{
private:
    std::string msg;

public:
    UnsupportedOperationException(const std::string& message = "") : msg(message)
    {
    }

    const char * what() const throw()
    {
        return msg.c_str();
    }
};
