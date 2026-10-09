// SPDX-License-Identifier: Apache-2.0
#ifndef ART_RUNTIME_HOST_SIGNAL_STACK_H_
#define ART_RUNTIME_HOST_SIGNAL_STACK_H_

#include <errno.h>
#include <stdint.h>
#include <signal.h>
#include <memory>
#include <new>

namespace art {

// Explicit per-thread lifecycle: install and tear down on the owning thread.
// Native app code can replace the kernel's stack without transferring ownership.
class HostSignalStack {
 public:
  bool Install(size_t size) {
    if (allocated_ != nullptr) { errno = EALREADY; return false; }
    std::unique_ptr<uint8_t[]> memory(new (std::nothrow) uint8_t[size]);
    if (!memory) { errno = ENOMEM; return false; }
    stack_t stack = {};
    stack.ss_sp = memory.get();
    stack.ss_size = size;
    if (sigaltstack(&stack, &previous_) != 0) return false;
    allocated_ = memory.release();
    return true;
  }

  bool TearDown() {
    if (allocated_ == nullptr) return true;
    stack_t current;
    if (sigaltstack(nullptr, &current) != 0) return false;
    if (current.ss_sp == allocated_ && !(current.ss_flags & SS_DISABLE)) {
      if (sigaltstack(&previous_, nullptr) != 0) return false;
    }
    delete[] allocated_;
    allocated_ = nullptr;
    return true;
  }

  HostSignalStack() = default;
  HostSignalStack(const HostSignalStack&) = delete;
  HostSignalStack& operator=(const HostSignalStack&) = delete;

 private:
  uint8_t* allocated_ = nullptr;
  stack_t previous_ = {};
};

}  // namespace art
#endif  // ART_RUNTIME_HOST_SIGNAL_STACK_H_
