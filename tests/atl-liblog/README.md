# Log output independent of app stderr

Run inside Alpine: `sh tests/atl-liblog/run.sh /absolute/path/to/liblog.so`.
The probe loads exactly the supplied library after assigning a test output file,
then replaces stderr with a nonblocking pipe. A log must reach the original
file and never the app pipe. It also checks descriptor close-on-exec. Library teardown is not asserted:
musl retains dlopened libraries until process exit. This prevents a logger that reads and re-emits stderr from
feeding its own output back indefinitely. No app or user logs are used.

Derived from OMP's four-app liblog fix (shared ART candidate 1ef6e1d3); integration
adds close-on-exec, teardown and deterministic tests. Existing AOSP license
headers are retained. Capture occurs at library load, not on the first message.
If the original descriptor cannot be duplicated, logging does not retry against
a potentially redirected stderr. This is a host output adapter, not Android logd.
