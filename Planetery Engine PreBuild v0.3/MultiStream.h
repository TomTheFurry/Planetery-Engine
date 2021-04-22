#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>

class MultiStream : public std::ostream {
    struct MultiBuffer : public std::streambuf {
        void addBuffer(std::streambuf* buf) {
            bufs.push_back(buf);
        }
        virtual int overflow(int c) {
            std::for_each(bufs.begin(), bufs.end(), std::bind(std::mem_fn(&std::streambuf::sputc), std::placeholders::_1, c));
            return c;
        }

    private:
        std::vector<std::streambuf*> bufs;

    };
    MultiBuffer myBuffer;
public:
    MultiStream() : std::ostream(&myBuffer) {}
    void linkStream(std::ostream& out) {
        out.flush();
        myBuffer.addBuffer(out.rdbuf());
    }
};

