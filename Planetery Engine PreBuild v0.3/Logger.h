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

class ScropedLayer;	   // Auto close on scrope exit
class UnscropedLayer;  // Use new() for creation
class ULayerSession;  // Notify entering a unscroped layer. close on scrope exit
class LayerTraces;    // A Layer Trace stack
class Info;			  // Only display on error/warning
class Message;		  // Std display.
class Warning;		  // Display also previous Info
class Error;		  // Display previous Info & source Loc & LayerTrace
class DynamicInfo;	  // Message with var attached
class ActiveAssert;	  // Actively check if value is correct
class TraceSession;	  // Activate Trace mode session. can set if use active
					  // flushing
//void checkpoint();	  // Display all info and change of LayerEntry/Exit if Trace
					  // mode is activated
//void assert(bool a);  // assert value. If fail throw Error
//void raise();		  // same as assert(false)
//void breakpoint();	  // trigger breakpoint


#include "ConsoleFormat.h"