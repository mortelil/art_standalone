# Explicit trust anchors

Inside atl_dev:

    CORE_JAR=/path/to/core-all-hostdex.jar RUNTIME_LIB=/path/to/runtime/lib sh tests/trust-anchors/run.sh

Builds the checkout's WolfSSL Java provider and synthetic certificates. Compares
OpenJDK PKIX with ART/wolfJSSE: explicit RSA/EC non-CA anchors and a valid CA chain
must pass; an untrusted certificate with the same subject but different key,
an empty trust store, a peer-supplied untrusted CA, and an invalid signature must
fail. No network services, user certificates or accounts are used.

This implements trust-store selection, not permissive verification. Native
WolfSSL still verifies signatures/chains. The probe halts after assertions to
avoid unrelated standalone runtime teardown; it does not verify VM shutdown.
