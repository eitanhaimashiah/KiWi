#pragma once

#include <stdexcept>

class IllegalStateException : public std::exception
{
private:
    std::string msg;

public:
    IllegalStateException(const std::string& message = "") : msg(message)
    {
    }

    const char * what() const throw()
    {
        return msg.c_str();
    }
};

class NoSuchElementException : public std::exception
{
private:
    std::string msg;

public:
    NoSuchElementException(const std::string& message = "") : msg(message)
    {
    }

    const char * what() const throw()
    {
        return msg.c_str();
    }
};

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
