#include <cinttypes>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <android/log.h>
#include <cstdarg>
#include <link.h>
#include <sys/auxv.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <vector>

#include "zygisk_next_api.h"

// An example module which inject to adbd and hook some functions

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "ZnModuleTemplate", __VA_ARGS__)

static ZygiskNextAPI api_table;

// print the arguments of programs that the adbd exec-ed
static int my_execle_plt(const char *pathname, char *arg, ...) {
    va_list va;
    std::vector<char*> args;
    LOGI("exec pathname=%s", pathname);
    args.push_back(arg);
    va_start(va, arg);
    char* next = arg;
    int i = 0;
    while (next) {
        LOGI("exec arg %d %s", i, next);
        i++;
        next = va_arg(va, char*);
        args.push_back(next);
    };
    auto envp = va_arg(va, char* const*);
    return execve(pathname, args.data(), envp);
}

// backup of old open function
static int (*old_open)(const char *pathname, int flags, mode_t mode) = nullptr;
// our replacement for open function
static int my_open(const char *pathname, int flags, mode_t mode) {
    auto fd = old_open(pathname, flags, mode);
    int e = errno;
    LOGI("open %s = %d", pathname, fd);
    errno = e;
    return fd;
}

// this function will be called after all of the main executable's needed libraries are loaded
// and before the entry of the main executable called
void onModuleLoaded(void* self_handle, const struct ZygiskNextAPI* api) {
    // You need to copy the api table if you want to use it after this callback finished
    memcpy(&api_table, api, sizeof(struct ZygiskNextAPI));
    LOGI("module loaded");

    // get base address of adbd
    void* base = nullptr;
    dl_iterate_phdr([](struct dl_phdr_info* info, size_t sz, void* data) -> int {
        // LOGI("%s base %" PRIxPTR, info->dlpi_name, info->dlpi_addr);
        auto linker_base = (uintptr_t) getauxval(AT_BASE);
        // LOGI("linker base=%" PRIxPTR, linker_base);
        if (linker_base == info->dlpi_addr)
            return 0;
        *reinterpret_cast<void**>(data) = (void*) info->dlpi_addr;
        return 1;
    }, &base);

    LOGI("adbd base %p", base);

    // plt hook adbd's execle function
    if (api_table.pltHook(base, "execle", (void*) my_execle_plt, nullptr) == ZN_SUCCESS) {
        LOGI("plt hook success");
    } else {
        LOGI("plt hook failed");
    }

    // inline hook adbd's open function
    auto fun = dlsym(RTLD_NEXT, "open");
    LOGI("open addr %p", fun);
    if (api_table.inlineHook(fun, (void *) my_open, (void**) &old_open) == ZN_SUCCESS) {
        LOGI("inline hook success %p", old_open);
    } else {
        LOGI("inline hook failed");
    }
}

// declaration of the zygisk next module
__attribute__((visibility("default"), unused))
struct ZygiskNextModule zn_module = {
        .target_api_version = ZYGISK_NEXT_API_VERSION_1,
        .onModuleLoaded = onModuleLoaded
};

