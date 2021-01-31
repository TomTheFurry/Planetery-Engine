#include "Logger.h"
#include "Define.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <mutex>
#include <unordered_map>
#include <thread>
#include <chrono>
#include "MultiStream.h"
#include "MultilineString.h"

#define LOGGER_DEFAULT_THREAD_NAME "UnknownThread"+format({BRIGHT COLOR_RED})+toStr(std::this_thread::get_id())
#define LOGGER_DEFAULT_THREAD_NAME_FORMAT format({RESET_ALL,UNDERLINE,BRIGHT COLOR_WHITE})
#define LOGGER_MESSAGE_START LOGGER_DEFAULT_THREAD_NAME_FORMAT << name << ":\n"
#define LOGGER_MESSAGE_END "\n"
#define LOGGER_NEWLINE_PADDING std::string(FORMAT_RESET)+"  "
#define LOGGER_MAX_LINE_WIDTH size_t(100) //set -1 as infinate
//WARN: This causes bugs. Many bugs. the MultiLineString impimentation needs to redo.


std::string& replaceStringInPlace(std::string&& subject, const std::string& search,
    const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
    return subject;
}

namespace Logger {
    class LogLayerBase {
    public:
        bool closed = false;
        LogLayerBase() = default;
        virtual ~LogLayerBase() = default;
        virtual MultilineString str() const { throw; }
        virtual LogLayerBase* getBack() { throw; }
        virtual bool push(LogLayerBase* object) { throw; }
        virtual bool pop() { throw; }
    };
    class LogLayerContainer : public LogLayerBase {
    public:
        LogLayerContainer() = default;
        virtual ~LogLayerContainer();
        virtual MultilineString str() const;
        virtual LogLayerBase* getBack();
        virtual bool push(LogLayerBase* object);
        virtual bool pop();
        std::vector<LogLayerBase*> childs;
    };
    class LogLayerString : public LogLayerBase {
    public:
        LogLayerString();
        virtual ~LogLayerString() = default;
        virtual MultilineString str() const;
        virtual LogLayerBase* getBack();
        virtual bool push(LogLayerBase* object);
        virtual bool pop();
        std::stringstream string;
    };
    class Message {
    public:
        Message(std::string&& str);
        Message();
        LogLayerContainer data;
        std::string name;
        friend std::ostream& operator<<(std::ostream& out, const Message& ms);
    };
    //OPTI: Make this thread_local to maybe save time and save the need to declare threads
    class SingleThread {
    public:
        SingleThread(std::mutex& mx, std::ostream& os, std::string&& name);
        std::mutex& lock;
        std::ostream& output;
        std::string name;
        std::vector<Message> messages;
        void newLayer();
        void closeLayer();
        void newMessage();
        void closeMessage();
        void newMessage(std::string&& str);
        void closeMessage(std::string&& str);
        std::ostream& getStream();
    };
    static std::unordered_map<std::thread::id, SingleThread> threads{};
    static std::ofstream fs{"log.txt"};
    static std::mutex writeLock{};
    static MultiStream outStream{};
}

namespace Logger {
    std::ostream& operator<<(std::ostream& os, const Message& ms) {
        return (os << ms.data.str());
    }
}

template <typename T>
inline std::string toStr(T t) {
    return (std::stringstream() << t).str();
}
inline std::string getTimestamp() {
    std::chrono::system_clock::time_point p = std::chrono::system_clock::now();
    time_t t = std::chrono::system_clock::to_time_t(p);
    char str[26];
    ctime_s(str, sizeof str, &t);
    return std::string(str);
}

Logger::LogLayerContainer::~LogLayerContainer() {
    for (auto c : childs) {
        //delete c;
    }
}

MultilineString Logger::LogLayerContainer::str() const {
    if (childs.empty()) return MultilineString();
    MultilineString ms{childs[0]->str(),LOGGER_MAX_LINE_WIDTH};
    for (size_t i = 1; i<childs.size(); i++) {
        ms << childs[i]->str();
    }
    ms.padLeft(LOGGER_NEWLINE_PADDING);
    return ms;
}

Logger::LogLayerBase* Logger::LogLayerContainer::getBack() {
    if (childs.empty() || childs.back()->closed) return this;
    return childs.back()->getBack();
}

bool Logger::LogLayerContainer::push(LogLayerBase* object) {
    if (closed) return false;
    for (size_t i = childs.size(); i!=0; i--) {
        if (childs[i-1]->push(object)) return true;
    }
    childs.push_back(object);
    return true;
}

bool Logger::LogLayerContainer::pop() {
    if (closed) return false;
    if (!childs.empty() && childs.back()->pop()) return true;
    closed = true;
    return true;
}

Logger::LogLayerString::LogLayerString() : string() {}

MultilineString Logger::LogLayerString::str() const {
    return MultilineString(format({BRIGHT COLOR_BLACK})+"-"+format({COLOR_DEFAULT})+string.str());
}

Logger::LogLayerBase* Logger::LogLayerString::getBack() {
    return dynamic_cast<LogLayerBase*>(this);
}

bool Logger::LogLayerString::push(LogLayerBase* object) {
    return false;
}

bool Logger::LogLayerString::pop() {
    return false;
}


Logger::Message::Message(std::string&& str) : name(str), data() {}

Logger::Message::Message() : Message(getTimestamp()) {}

Logger::SingleThread::SingleThread(
    std::mutex& mx, std::ostream& os,
    std::string&& str = LOGGER_DEFAULT_THREAD_NAME
) : lock(mx), output(os), name(str) {}

void Logger::SingleThread::newLayer() {
    if (messages.empty()) {
        messages.emplace_back();
    } else {
        if (!messages.back().data.push(new LogLayerContainer())) {
            messages.emplace_back();
        }
    }
}
void Logger::SingleThread::closeLayer() {
    messages.back().data.pop();
    if (messages.back().data.closed) {
        lock.lock();
        output << LOGGER_MESSAGE_START << messages.back() << LOGGER_MESSAGE_END;
        //output << name << ":\n" << messages.back() << "\n";
        output.flush();
        lock.unlock();
        messages.pop_back();
    }
}

void Logger::SingleThread::newMessage() {
    messages.emplace_back();
}

void Logger::SingleThread::closeMessage() {
    lock.lock();
    output << LOGGER_MESSAGE_START << messages.back() << LOGGER_MESSAGE_END;
    //output << name << ":\n" << messages.back() << "\n";
    output.flush();
    lock.unlock();
    messages.pop_back();
}

void Logger::SingleThread::newMessage(std::string&& str) {
    messages.emplace_back(std::move(str));
}

void Logger::SingleThread::closeMessage(std::string&& str) {
    if (messages.back().name!=str) throw "Logger: TopMessage not the correct one!";
    lock.lock();
    output << LOGGER_MESSAGE_START << messages.back() << LOGGER_MESSAGE_END;
    //output << name << ":\n" << messages.back() << "\n";
    output.flush();
    lock.unlock();
    messages.pop_back();
}

std::ostream& Logger::SingleThread::getStream() {
    auto back = (messages.back().data.getBack());
    auto* pt = dynamic_cast<LogLayerString*>(back);
    if (pt==nullptr) {
        auto backContainer = dynamic_cast<LogLayerContainer*>(back);
        auto newString = new LogLayerString();
        if (!backContainer->push(newString)) throw "Logger: Failed to insert new string!";
        return newString->string;
    }
    return (dynamic_cast<LogLayerString*>((messages.back().data.getBack()))->string);
}

void Log::newThread() {
    auto r = Logger::threads.try_emplace(std::this_thread::get_id(), Logger::writeLock, Logger::outStream);
    if (!r.second) throw "Logger: Thread already exists.";
}

void Log::closeThread() {
    auto i = Logger::threads.erase(std::this_thread::get_id());
    if (i!=1) throw "Logger: Thread already closed.";
}

void Log::newThread(std::string&& str) {
    auto r = Logger::threads.try_emplace(std::this_thread::get_id(), Logger::writeLock, Logger::outStream, std::move(str));
    if (!r.second) throw "Logger: Thread already exists.";
}

void Log::closeThread(std::string&& str) {
    auto it = Logger::threads.find(std::this_thread::get_id());
    if (it == Logger::threads.end()) throw "Logger: Thread already closed.";
    if (it->second.name != str) throw "Logger: Thread name does not match!";
    Logger::threads.erase(it);
}

void Log::newLayer() {
    auto r = Logger::threads.try_emplace(std::this_thread::get_id(), Logger::writeLock, Logger::outStream);
    r.first->second.newLayer();
}

void Log::closeLayer() {
    Logger::threads.at(std::this_thread::get_id()).closeLayer();
}

void Log::newMessage() {
    auto r = Logger::threads.try_emplace(std::this_thread::get_id(), Logger::writeLock, Logger::outStream);
    r.first->second.newMessage();
}

void Log::closeMessage() {
    Logger::threads.at(std::this_thread::get_id()).closeMessage();
}

void Log::newMessage(std::string&& str) {
    auto r = Logger::threads.try_emplace(std::this_thread::get_id(), Logger::writeLock, Logger::outStream, "Testing");
    r.first->second.newMessage(std::move(str));
}

void Log::closeMessage(std::string&& str) {
    Logger::threads.at(std::this_thread::get_id()).closeMessage(std::move(str));
}

Log::Log() {
    Logger::outStream.linkStream(std::cout);
    Logger::outStream.linkStream(Logger::fs);
}

std::ostream& Log::getStream() {
    return Logger::threads.at(std::this_thread::get_id()).getStream();
}

extern Log logger = Log{};

