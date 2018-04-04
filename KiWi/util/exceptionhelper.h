#ifndef exceptionhelper_h
#define exceptionhelper_h

#include <stdexcept>

using namespace std;

class IllegalStateException : public exception
{
private:
    string msg;
    
public:
    IllegalStateException(const string& message = "") : msg(message)
    {
    }
    
    const char * what() const throw()
    {
        return msg.c_str();
    }
};

class NoSuchElementException : public exception
{
private:
    string msg;
    
public:
    NoSuchElementException(const string& message = "") : msg(message)
    {
    }
    
    const char * what() const throw()
    {
        return msg.c_str();
    }
};

class NotImplementedException : public exception
{
private:
    string msg;

public:
    NotImplementedException(const string& message = "") : msg(message)
    {
    }

    const char * what() const throw()
    {
        return msg.c_str();
    }
};

class UnsupportedOperationException : public exception
{
private:
    string msg;

public:
    UnsupportedOperationException(const string& message = "") : msg(message)
    {
    }

    const char * what() const throw()
    {
        return msg.c_str();
    }
};

#endif /* exceptionhelper_h */
