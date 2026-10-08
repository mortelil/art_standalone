// SPDX-License-Identifier: Apache-2.0
#include <utils/Looper.h>
#include <utils/Timers.h>
#include <poll.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <cassert>
#include <cstdio>
using namespace android;
extern "C" void ATL_Looper_getPollState(Looper*, int*, int64_t*);
static int callbacks;
static int input(int fd, int, void*) {
    uint64_t value;
    assert(read(fd, &value, sizeof(value)) == sizeof(value));
    ++callbacks;
    return 0; // must unregister the descriptor
}
struct Handler : MessageHandler {
    void handleMessage(const Message&) override { ++callbacks; }
};
int main() {
    auto looper = Looper::prepare(0);
    int fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    assert(fd >= 0);
    assert(looper->addFd(fd, 0, Looper::EVENT_INPUT, input, nullptr) == 1);
    looper->pollOnce(0);
    int epoll_fd; int64_t deadline;
    ATL_Looper_getPollState(looper.get(), &epoll_fd, &deadline);
    assert(deadline == -1);
    pollfd p = {epoll_fd, POLLIN, 0};
    assert(poll(&p, 1, 0) == 0);
    uint64_t one = 1;
    assert(write(fd, &one, sizeof(one)) == sizeof(one));
    assert(poll(&p, 1, 100) == 1);
    looper->pollOnce(0);
    assert(callbacks == 1);
    assert(write(fd, &one, sizeof(one)) == sizeof(one));
    assert(poll(&p, 1, 0) == 0); // callback removal respected
    sp<Handler> handler = new Handler();
    looper->sendMessageDelayed(30000000, handler, Message(1));
    assert(poll(&p, 1, 100) == 1); // queued message wakes external loop
    looper->pollOnce(0);
    ATL_Looper_getPollState(looper.get(), &epoll_fd, &deadline);
    assert(deadline > systemTime(SYSTEM_TIME_MONOTONIC) / 1000);
    while (systemTime(SYSTEM_TIME_MONOTONIC) / 1000 < deadline) usleep(1000);
    looper->pollOnce(0);
    assert(callbacks == 2);
    ATL_Looper_getPollState(looper.get(), &epoll_fd, &deadline);
    assert(deadline == -1);
    close(fd);
    puts("PASS: native fd readiness, callback removal, wakeup, delayed message deadline, idle state");
}
