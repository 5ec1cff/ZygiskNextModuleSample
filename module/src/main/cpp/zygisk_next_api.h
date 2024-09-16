#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define ZYGISK_NEXT_API_VERSION_1 1

#define ZN_SUCCESS 0
#define ZN_FAILED 1

struct ZygiskNextAPI {
    int (*pltHook)(void* base_addr, const char* symbol, void* hook_handler, void** original);
    int (*inlineHook)(void* target, void* addr, void** original);
    int (*inlineUnhook)(void* target);
};

struct ZygiskNextModule {
    int target_api_version;
    void (*onModuleLoaded)(void* self_handle, const struct ZygiskNextAPI* api);
};

extern __attribute__((visibility("default"), unused)) struct ZygiskNextModule zn_module;

#ifdef __cplusplus
}
#endif
