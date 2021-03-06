#pragma once

#include <utility>
#include <atomic>

namespace sync {
    template<typename T> class Promisee;
    template<typename T> class Promisor;
    template<typename T>
    std::pair<Promisee<T>, Promisor<T>> newPromisedData(T&& data);
    template<typename T> class Promisee
    {
        std::atomic<bool>* _c;
        std::atomic<T>* _d;
      protected:
        Promisee(std::atomic<bool>* c, std::atomic<T>* d) noexcept:
          _c(c), _d(d) {}
      public:
        Promisee(const Promisee& p) = delete;
        Promisee(Promisee&& p) noexcept: _c(p._c), _d(p._d) { p._c = nullptr; };
        const std::atomic<bool>* operator->() const noexcept { return _c; }
        // Will block if thread haven't gotten the acknowledgement yet.
        ~Promisee() noexcept {
            if (_c != nullptr) {
                _c->wait(false, std::memory_order_relaxed);
                delete _c;
                delete _d;
            }
        };
        template<typename U>
        friend std::pair<Promisee<U>, Promisor<U>> newPromisedData<U>(U&& data);
        // friend std::pair<Promisee<T>, Promisor<T>> newPromisedData<T>(T&&
        // data);
    };
    template<> class Promisee<bool>
    {
        std::atomic<bool>* _c;
      protected:
        Promisee(std::atomic<bool>* c) noexcept: _c(c) {}
      public:
        Promisee(const Promisee<bool>& p) = delete;
        Promisee(Promisee<bool>&& p) noexcept: _c(p._c) { p._c = nullptr; }
        const std::atomic<bool>* operator->() const noexcept { return _c; }
        // Will block if thread haven't gotten the acknowledgement yet.
        ~Promisee() noexcept {
            if (_c != nullptr) {
                _c->wait(false, std::memory_order_relaxed);
                delete _c;
            }
        }
        friend std::pair<Promisee<bool>, Promisor<bool>> newPromises();
    };

    template<typename T> class Promisor
    {
        std::atomic<bool>* _c;
        std::atomic<T>* _d;
      protected:
        Promisor(std::atomic<bool>* c, std::atomic<T>* d) noexcept:
          _c(c), _d(d) {}
      public:
        Promisor(const Promisor<T>& p) = delete;
        Promisor(Promisor<T>&& p) noexcept: _c(p._c), _d(p._d) {
            p._c = nullptr;
        }
        void sendPromise() noexcept {
            _c->store(true, std::memory_order_relaxed);
            _c = nullptr;
        };
        // Will send promise if not done already.
        ~Promisor() noexcept {
            if (_c != nullptr) { _c->store(true, std::memory_order_relaxed); }
        }
        template<typename U>
        friend std::pair<Promisee<U>, Promisor<U>> newPromisedData<U>(U&& data);
        // friend std::pair<Promisee<T>, Promisor<T>> newPromisedData<T>(T&&
        // data);
    };
    template<> class Promisor<bool>
    {
        std::atomic<bool>* _c;
      protected:
        Promisor(std::atomic<bool>* c) noexcept: _c(c) {}
      public:
        Promisor(const Promisor& p) = delete;
        Promisor(Promisor&& p) noexcept: _c(p._c) { p._c = nullptr; }
        void sendPromise() noexcept {
            _c->store(true, std::memory_order_relaxed);
            _c = nullptr;
        }
        // Will send promise if not done already.
        ~Promisor() noexcept {
            if (_c != nullptr) { _c->store(true, std::memory_order_relaxed); }
        }
        friend std::pair<Promisee<bool>, Promisor<bool>> newPromises();
    };
    std::pair<Promisee<bool>, Promisor<bool>> newPromises();

    template<typename T>
    std::pair<Promisee<T>, Promisor<T>> newPromisedData(T&& data) {
        auto* ptr = new std::atomic<bool>(false);
        auto* dPtr = new std::atomic<T>(std::forward<T>(data));
        return std::make_pair(Promisee<T>(ptr, dPtr), Promisor<T>(ptr, dPtr));
    }



}
