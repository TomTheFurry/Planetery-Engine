#include "DefineThreadSync.h"

using namespace sync;

std::pair<Promisee<bool>, Promisor<bool>> sync::newPromises() {
    auto* ptr = new std::atomic<bool>(false);
    return std::make_pair(Promisee<bool>(ptr), Promisor<bool>(ptr));
}