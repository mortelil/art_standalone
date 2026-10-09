/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "thread.h"
#include "host_signal_stack.h"

#include <signal.h>

#include "base/logging.h"  // For VLOG.
#include "base/utils.h"

namespace art {

static void SigAltStack(stack_t* new_stack, stack_t* old_stack) {
  if (sigaltstack(new_stack, old_stack) == -1) {
    PLOG(FATAL) << "sigaltstack failed";
  }
}

// The default SIGSTKSZ on linux is 8K.  If we do any logging in a signal
// handler or do a stack unwind, this is too small.  We allocate 32K
// instead of the minimum signal stack size.
// TODO: We shouldn't do logging (with locks) in signal handlers.
static /*constexpr*/ int kHostAltSigStackSize =
    32 * KB < MINSIGSTKSZ ? MINSIGSTKSZ : 32 * KB;

// Keep host-only ownership outside Thread's Android-compatible layout.
static thread_local HostSignalStack owned_signal_stack;
static thread_local const Thread* signal_stack_owner = nullptr;

void Thread::SetUpAlternateSignalStack() {
#ifdef ART_TARGET_ANDROID
  LOG(FATAL) << "Invalid use of alternate signal stack on Android";
#endif
  if (!owned_signal_stack.Install(kHostAltSigStackSize)) {
    PLOG(FATAL) << "alternate signal stack installation failed";
  }
  signal_stack_owner = this;
  stack_t ss;
  SigAltStack(nullptr, &ss);
  VLOG(threads) << "Alternate signal stack is " << PrettySize(ss.ss_size) << " at " << ss.ss_sp;
}

void Thread::TearDownAlternateSignalStack() {
  // A Thread whose native creation failed never installed a signal stack.
  if (signal_stack_owner != this) return;
  if (!owned_signal_stack.TearDown()) {
    PLOG(FATAL) << "alternate signal stack teardown failed";
  }
  signal_stack_owner = nullptr;
}

}  // namespace art
