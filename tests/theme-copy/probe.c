/* SPDX-License-Identifier: Apache-2.0 */
#include <androidfw/androidfw_c_api.h>
#include <assert.h>
#include <stdio.h>
static struct AssetManager *manager(const struct ApkAssets **assets, size_t count, int night) {
    struct AssetManager *am = AssetManager_new();
    assert(AssetManager_setApkAssets(am, assets, count, true, true));
    struct ResTable_config config = {.size=sizeof(config), .density=160, .sdkVersion=28, .uiMode=night?0x20:0x10};
    AssetManager_setConfiguration(am, &config);
    return am;
}
static uint32_t attr(struct AssetManager *am, const char *name, const char *package) {
    uint32_t id = AssetManager_getResourceId(am, name, "attr", package);
    assert(id); return id;
}
static void value(struct Theme *theme, uint32_t id, uint32_t expected) {
    struct Res_value v; uint32_t flags;
    assert(Theme_getAttribute(theme,id,&v,&flags)>=0);
    assert(v.data==expected);
}
int main(int argc, char **argv) {
    assert(argc==3);
    const struct ApkAssets *assets[]={ApkAssets_load(argv[1],false),ApkAssets_load(argv[2],false)};
    assert(assets[0]&&assets[1]);
    struct AssetManager *day=manager(assets,2,0), *night=manager(assets,2,1);
    uint32_t style=AssetManager_getResourceId(day,"Probe","style","org.atl.themetest");
    uint32_t cross=attr(day,"crossPackage","org.atl.themetest");
    uint32_t local=attr(day,"localValue","org.atl.themetest");
    uint32_t accent=attr(day,"colorAccent","android");
    assert(style);
    struct Theme *source=AssetManager_newTheme(day), *dest=AssetManager_newTheme(night);
    Theme_applyStyle(source,style,true);
    Theme_applyStyle(dest,style,true);
    value(source,cross,0xff123456); value(source,local,7);
    value(dest,cross,0xffabcdef); value(dest,local,9);
    Theme_setTo(dest,source);
    value(dest,cross,0xff123456); value(dest,local,7); value(dest,accent,0xff123456);
    value(source,cross,0xff123456); value(source,local,7);
    Theme_setTo(dest,dest); value(dest,cross,0xff123456);
    struct Theme *same=AssetManager_newTheme(day);
    Theme_setTo(same,source); value(same,cross,0xff123456);
    struct Theme *empty=AssetManager_newTheme(night);
    Theme_setTo(dest,empty);
    struct Res_value v; uint32_t flags;
    assert(Theme_getAttribute(dest,cross,&v,&flags)<0);
    /* A destination without the app APK must not acquire app attributes,
     * even though its shared framework still has the same numeric IDs. */
    struct AssetManager *framework=manager(assets,1,0);
    struct Theme *limited=AssetManager_newTheme(framework);
    Theme_setTo(limited,source);
    assert(Theme_getAttribute(limited,cross,&v,&flags)<0);
    Theme_delete(limited);Theme_delete(empty);Theme_delete(same);Theme_delete(dest);Theme_delete(source);
    puts("PASS: cross-package references, source values across night configuration, self/same copy, clear and missing package");
}
