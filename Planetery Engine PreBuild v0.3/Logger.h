#pragma once
#include <string>
#include <iostream>
#include <functional>

class Log
{
  public:
    void newThread();
    void closeThread();
    void newThread(std::string&& str);
    void closeThread(std::string&& str);
    void newLayer();
    void closeLayer();
    void newMessage();
    void closeMessage();
    void newMessage(std::string&& str);
    void closeMessage(std::string&& str);
    template<typename T> std::ostream& operator<<(T t) {
        return (getStream() << t);
    }
    Log();
    template<typename T, typename... Args> void _loopStream(T t, Args... a) {
        *this << std::forward<T>(t);
        this->_loopStream(a...);
    }
    template<typename T> void _loopStream(T t) { *this << std::forward<T>(t); }
    template<typename... Args> void operator()(Args... a) {
        this->newLayer();
        this->_loopStream(a...);
        this->closeLayer();
    }
  private:
    std::ostream& getStream();
};

extern Log logger;

#include "ConsoleFormat.h"