# Host alternate signal-stack ownership

ART's Linux runtime allocates an alternate signal stack for each attached thread.
Native application code may subsequently install a different stack. Reading the
current kernel stack at ART teardown and deleting its address can therefore free
memory ART never owned (for example an mmap-backed Breakpad stack).

`HostSignalStack` records its allocation and the prior kernel stack. Teardown
restores the prior stack only if ART's allocation is still active, and releases
only ART's allocation. A replacement or explicitly disabled stack remains
unchanged. Setup and teardown must occur on the owning native thread; this is
an explicit lifecycle, not a process-wide manager. Host-only thread-local storage
keeps this bookkeeping out of Android's `Thread` layout. The owner check also
protects a parent's stack when a Thread object failed native creation.

Run `sh tests/mobile/run.sh` inside Alpine. The test delivers real SIGUSR1 signals
with SA_ONSTACK and checks the handler's stack address. It covers repeated
install/teardown, prior-stack restoration, replacement with mmap storage, disabled
stacks, failed installation and duplicate teardown. The full ATL smoke suite
additionally creates and tears down Java threads and captures Java stack traces.
ARM64 has not yet been exercised for this change.

Diagnosed and implemented with AI assistance. Existing AOSP license headers in
modified ART files are preserved; the ownership helper and tests are new code.
