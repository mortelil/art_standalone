// SPDX-License-Identifier: Apache-2.0
#include "art/runtime/host_signal_stack.h"
#include <assert.h>
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>

static volatile sig_atomic_t on_expected_stack;
static uintptr_t expected_start;
static size_t expected_size;
static void handler(int) {
  volatile char marker;
  uintptr_t address = reinterpret_cast<uintptr_t>(&marker);
  on_expected_stack = address >= expected_start && address < expected_start + expected_size;
}
static void check_signal(const stack_t& stack) {
  expected_start = reinterpret_cast<uintptr_t>(stack.ss_sp);
  expected_size = stack.ss_size;
  on_expected_stack = 0;
  assert(raise(SIGUSR1) == 0 && on_expected_stack);
}
int main() {
  stack_t original; assert(sigaltstack(nullptr, &original) == 0);
  struct sigaction action = {}, old_action;
  action.sa_handler = handler; action.sa_flags = SA_ONSTACK;
  assert(sigaction(SIGUSR1, &action, &old_action) == 0);
  const size_t size = 128 * 1024;
  void* external = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  assert(external != MAP_FAILED);
  stack_t foreign = {}; foreign.ss_sp = external; foreign.ss_size = size;
  // An attached native thread can already own a signal stack.
  assert(sigaltstack(&foreign, nullptr) == 0);
  for (int i = 0; i < 20; ++i) {
    art::HostSignalStack owned;
    assert(owned.Install(size));
    assert(!owned.Install(size) && errno == EALREADY);
    stack_t current; assert(sigaltstack(nullptr, &current) == 0);
    assert(current.ss_sp != external); check_signal(current);
    assert(owned.TearDown()); assert(owned.TearDown());
    assert(sigaltstack(nullptr, &current) == 0 && current.ss_sp == external);
    check_signal(current);
  }
  art::HostSignalStack failed;
  assert(!failed.Install(1) && errno == ENOMEM);
  assert(failed.TearDown());
  stack_t unchanged; assert(sigaltstack(nullptr, &unchanged) == 0 && unchanged.ss_sp == external);
  // Replacing ART's stack does not transfer the replacement's ownership.
  art::HostSignalStack replaced;
  assert(replaced.Install(size));
  assert(sigaltstack(&foreign, nullptr) == 0);
  assert(replaced.TearDown());
  stack_t current; assert(sigaltstack(nullptr, &current) == 0 && current.ss_sp == external);
  check_signal(current);
  // Explicitly disabling a stack before teardown is also supported.
  art::HostSignalStack disabled; assert(disabled.Install(size));
  stack_t off = {}; off.ss_flags = SS_DISABLE;
  assert(sigaltstack(&off, nullptr) == 0 && disabled.TearDown());
  assert(sigaltstack(nullptr, &current) == 0 && (current.ss_flags & SS_DISABLE));
  // Starting with no alt stack must restore the disabled state.
  art::HostSignalStack fresh; assert(fresh.Install(size) && fresh.TearDown());
  assert(sigaltstack(nullptr, &current) == 0 && (current.ss_flags & SS_DISABLE));
  assert(sigaltstack(&original, nullptr) == 0);
  assert(sigaction(SIGUSR1, &old_action, nullptr) == 0);
  assert(munmap(external, size) == 0);
  puts("PASS: actual alternate-stack delivery, prior-stack restoration, foreign replacement ownership, disabled stack and repeated teardown");
}
