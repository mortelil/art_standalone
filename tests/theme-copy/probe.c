// SPDX-License-Identifier: Apache-2.0
// Standalone C API regression test; fixtures are the ATL theme-override APK
// and the matching framework-res.apk. No Android app/account is launched.
#include <androidfw/androidfw_c_api.h>
#include <assert.h>
#include <stdio.h>

int main(int argc, char **argv) {
    assert(argc == 3);
    const struct ApkAssets *assets[] = {ApkAssets_load(argv[1], true), ApkAssets_load(argv[2], false)};
    assert(assets[0] && assets[1]);
    struct AssetManager *src = AssetManager_new(), *dst = AssetManager_new();
    assert(AssetManager_setApkAssets(src, assets, 2, true, true));
    assert(AssetManager_setApkAssets(dst, assets, 2, true, true));
    struct ResTable_config config = {.sdkVersion = 28, .density = 160};
    AssetManager_setConfiguration(src, &config);
    config.uiMode = 0x20; // UI_MODE_NIGHT_YES: copying must not mutate the source.
    AssetManager_setConfiguration(dst, &config);
    unsigned style = AssetManager_getResourceId(src, "InheritedStyle", "style", "org.atl.themeoverride");
    assert(style);
    struct Theme *a = AssetManager_newTheme(src), *b = AssetManager_newTheme(dst);
    Theme_applyStyle(a, style, true);
    Theme_setTo(b, a);
    const char *names[] = {"inheritedLabel", "inheritedFlag", "frameworkIcon"};
    for (unsigned i = 0; i < 3; i++) {
        unsigned id = AssetManager_getResourceId(src, names[i], "attr", "org.atl.themeoverride");
        struct Res_value av = {0}, bv = {0}; unsigned af = 0, bf = 0;
        int ac = Theme_getAttribute(a, id, &av, &af), bc = Theme_getAttribute(b, id, &bv, &bf);
        if (ac < 0 || bc != ac || av.dataType != bv.dataType || av.data != bv.data || af != bf) {
            fprintf(stderr, "FAIL %s source-cookie=%d destination-cookie=%d\n", names[i], ac, bc);
            return 1;
        }
    }
    unsigned flag = AssetManager_getResourceId(src, "inheritedFlag", "attr", "org.atl.themeoverride");
    struct Res_value v = {0}; unsigned f = 0;
    Theme_setTo(b, b);
    assert(Theme_getAttribute(b, flag, &v, &f) >= 0);
    struct Theme *empty = AssetManager_newTheme(src);
    Theme_setTo(b, empty);
    assert(Theme_getAttribute(b, flag, &v, &f) < 0);
    assert(Theme_getAttribute(a, flag, &v, &f) >= 0);
    Theme_delete(empty); Theme_delete(a); Theme_delete(b);
    puts("THEME_COPY_PASS shared assets cross-package references self-copy replacement source isolation");
}
