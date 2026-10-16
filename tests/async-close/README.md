# Host asynchronous close regression

Run `sh tests/async-close/run.sh LIBRARY_DIRECTORY OUTPUT_DIRECTORY` inside
Alpine against the newly built libnativehelper.so. The test blocks two threads
on a pipe read, signals monitors for that descriptor, and verifies both return
EINTR while an unrelated descriptor's monitor remains unsignaled.

On musl the former hardcoded signal 34 cannot be registered with sigaction,
but pthread_kill can deliver it, terminating the process. The old installed
library exits with signal 34 in this test. The host build uses SIGRTMIN + 1,
leaving SIGRTMIN for bionic_translation libbacktrace and +2 for ART timeout.
Bionic's original allocation is unchanged. Host libc signal allocation matters:
this must not be replaced with a hardcoded Linux signal number.

This covers the native monitor contract and was also checked with AntennaPod
podcast search on x86_64 Alpine. It has not been exercised on ARM in this change.
