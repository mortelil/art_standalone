// SPDX-License-Identifier: Apache-2.0
#include <nativehelper/AsynchronousCloseMonitor.h>
#include <atomic>
#include <cassert>
#include <cerrno>
#include <chrono>
#include <thread>
#include <unistd.h>
#include <cstdio>

int main() {
    AsynchronousCloseMonitor::init();
    int fds[2];
    assert(pipe(fds) == 0);
    std::atomic<int> ready{0}, done{0};
    auto reader = [&] {
        AsynchronousCloseMonitor monitor(fds[0]);
        ++ready;
        char byte;
        int rc = read(fds[0], &byte, 1);
        assert(rc == -1 && errno == EINTR);
        assert(monitor.wasSignaled());
        ++done;
    };
    std::thread a(reader), b(reader);
    while (ready != 2) std::this_thread::yield();
    AsynchronousCloseMonitor unrelated(fds[1]);
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
    // Repeat to cover the legitimate gap between registering and entering read.
    while (done != 2 && std::chrono::steady_clock::now() < deadline) {
        AsynchronousCloseMonitor::signalBlockedThreads(fds[0]);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    assert(done == 2);
    assert(!unrelated.wasSignaled());
    a.join(); b.join();
    close(fds[0]); close(fds[1]);
    puts("PASS: asynchronous close interrupts both readers, leaves unrelated FD alone");
}
