# Theme copy across configuration AssetManagers

`Theme::SetTo` must preserve stored values and cookies when both managers use
exactly the same ordered ApkAssets objects. Configuration selection remains
independent. In particular an application theme attribute may point to a
framework drawable; the previous package-remapping path dropped this value.
The fix uses a verbatim copy only for identical asset lists and leaves the
existing remapping path in place for different lists.

`probe.c` checks cross-package references, app references, booleans, self-copy,
replacement by an empty theme, and source isolation. The fixtures come from
ATL's `tests/theme-override` (including `frameworkIcon`) and its matching
`framework-res.apk`. Build that APK using its `build.sh` in Alpine.

Example (inside Alpine; adjust directories):

```sh
make -j4 libandroidfw
cc -D_LARGEFILE64_SOURCE tests/theme-copy/probe.c \
  -Ilibandroidfw/include -L/usr/lib/art -Wl,-rpath,/usr/lib/art \
  -landroidfw -lpng -o /tmp/theme-copy-probe
LD_LIBRARY_PATH="$PWD/out/host/linux-x86/lib64:/usr/lib/art" \
  /tmp/theme-copy-probe "$ATL_BUILD_DIR/res/framework-res/framework-res.apk" \
  "$PROBE_DIR/theme-override.apk"
```

For a negative control use the same executable and fixtures with
`LD_LIBRARY_PATH=/usr/lib/art`. Alpine libandroidfw 0_git20251009-r2 fails
`frameworkIcon` (destination cookie -1); the patched build passes.

The existing AOSP Apache-2.0 source/license headers are retained. This small
fast path is new code in the existing AOSP Theme implementation, not a new
implementation of general resource/package remapping. The full upstream native
test suite and ARM runtime have not been run for this change.
