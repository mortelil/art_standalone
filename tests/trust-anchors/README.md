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

Cross-signed roots: when a peer-supplied CA has both the subject and public key
of a configured anchor, validation terminates at that anchor instead of requiring
its alternate issuer. The remaining chain still undergoes native verification.
Tests include an absent alternate issuer, a redundant alternate path, different
trusted keys with the same name, empty trust stores, expired and forged leaves.
This is bounded anchor recognition, not a replacement for a full PKIX path builder.
Reference: Conscrypt TrustManagerImpl.findTrustAnchorBySubjectAndPublicKey,
platform/external/conscrypt commit de55e62f6c7ecd57d0a91f2b497885c3bdc661d3.
No Conscrypt code copied; changes are AI-assisted.
