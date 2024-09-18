#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ZYGISK_NEXT_API_VERSION_1 2

#define ZN_SUCCESS 0
#define ZN_FAILED 1

struct ZnSymbolResolver;

struct ZygiskNextAPI {
    // Hook API

    // Do plt hook at symbol specified by the param `symbol` of library specified by the param `base_addr`
    // The plt address of `symbol` in the library will be replaced with hook_handler, and
    // its original value will be put to the address specified by `origianl` (can be null).
    // You can use this api to do caller-oriented hook
    // If you want to unhook, please call this function with hook_handler = original
    // If hooking succeed, returns ZN_SUCCESS, otherwise ZN_FAILED
    int (*pltHook)(void* base_addr, const char* symbol, void* hook_handler, void** original);

    // Do inline hook at the address specified by `target`, replace it with a new function specified
    // by `addr`, and the param `original` receives the address of original function.
    // You can use this api to achieve a global hook in current process.
    // In the current implementation , an address can only hook once, so the module can't hook an
    // address which is already hooked by an another module, except that the module unhooked it.
    // If hooking succeed, returns ZN_SUCCESS, otherwise ZN_FAILED
    int (*inlineHook)(void* target, void* addr, void** original);

    // Unhook the address which is formerly hooked.
    // If hooking succeed, returns ZN_SUCCESS, otherwise ZN_FAILED
    int (*inlineUnhook)(void* target);

    // Symbol Resolver API

    // Obtain a new ZnSymbolResolver object
    // `path` is required, which specifies the path of library to resolve. It can be an absolute path
    // or just the file name of library, e.g. /system/lib64/libc.so or libc.so .
    // If `base_addr` is non-zero, it will be used as the base address of the library.
    // Otherwise, Zygisk Next will try to find out the base address of the specified library in this process.
    // If succeed, it returns a valid pointer to the symbol resolver, otherwise nullptr is returned.
    struct ZnSymbolResolver* (*newSymbolResolver)(const char* path, void* base_addr);

    // Release the ZnSymbolResolver object pointed by `resolver`.
    void (*freeSymbolResolver)(struct ZnSymbolResolver* resolver);

    // Retrieve the base address of the library of the resolver image in the process.
    void* (*getBaseAddress)(struct ZnSymbolResolver* resolver);

    // Lookup the address of symbol by name or prefix (if `prefix` is true)
    // If the symbol exists, the function returns its address, otherwise returns nullptr.
    // If `size` is not nullptr, the size of the symbol will be put to *size .
    // In the current implementation, gnu_debugdata resolution is supported.
    void* (*symbolLookup)(struct ZnSymbolResolver* resolver, const char* name, bool prefix, size_t* size);

    // Walk through the symbol table of the library, the callback will receive the name, the address,
    // and the size of each symbol. Returning false in the callback means stop the walking.
    void (*forEachSymbols)(struct ZnSymbolResolver* resolver,
                           bool (*callback)(const char* name, void* addr, size_t size, void* data),
                           void* data);
};

struct ZygiskNextModule {
    // Please fill this with the target version of your module, e.g. ZYGISK_NEXT_API_VERSION_1
    int target_api_version;

    // This callback will be called after all needed library of the main executable are loaded,
    // and before the entry (i.e. `main`) of the main executable is called.
    void (*onModuleLoaded)(void* self_handle, const struct ZygiskNextAPI* api);
};

// Please define your `zn_module` in your source file.
extern __attribute__((visibility("default"), unused)) struct ZygiskNextModule zn_module;

#ifdef __cplusplus
}
#endif
