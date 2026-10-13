# Cross-package Theme::SetTo

Run inside Alpine:

```
sh tests/theme-copy/run.sh /path/to/built/lib64 /path/to/framework-res.apk /existing/test-output
```

Builds its resource-only APK locally; no third-party APK is committed. The test
uses the exact supplied native library directory and the public androidfw C API.
An app style references ?android:attr/colorAccent. Copying between AssetManagers
must preserve that reference and source values, including when the destination
selects a different day/night style. Reapplying the style there would produce
different values and fails this test. Self-copy, same-manager copy, empty-source
clearing, source preservation and a destination missing the app package are also
checked. All must pass; the original library fails the cross-package assertion.

The root cause was identified while reviewing OMP Sketchbook f81a24b's theme
replay workaround. This patch changes only the package lookup for reference
values; it retains the shared-ApkAssets requirement and all existing cookie and
attribute remapping. Adaptation is in the existing AOSP Apache-2.0 file,
libandroidfw/AssetManager2.cpp (base 327b9c2e). No claim of general overlay or
multi-package compatibility coverage is made by this small fixture.
