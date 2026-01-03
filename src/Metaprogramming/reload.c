#include "reload.h"

#include <assert.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include "reload.h"
#include <dlfcn.h>
#include <sys/stat.h>
#else
#error "Unsupported platform"
#endif


#ifdef _WIN32
typedef HMODULE lib_handle_t;
#define LOAD_SYMBOL_IMPL(handle, name) GetProcAddress(handle, name)
#elif __linux__
static struct timespec mod_time;
typedef void *lib_handle_t;
#define LOAD_SYMBOL_IMPL(handle, name) dlsym(handle, name)
#else
#error "Platform not supported"
#endif

// FIXME : Hack. remove the return or make a function.
#define LOAD_SYMBOL_REQUIRED(library, name)                                                                            \
    do {                                                                                                               \
        library->name = (name##_t)LOAD_SYMBOL_IMPL(library->lib_handle, #name);                                        \
        if(!library->name) {                                                                                           \
            fprintf(stderr, "[CRITICAL] Could not find symbol %s\n", dlerror());                                       \
            return 1;                                                                                                  \
        }                                                                                                              \
    } while(0);

#define LOAD_SYMBOL_OPTIONAL(library, name)                                                                            \
    do {                                                                                                               \
        library->name = (name##_t)LOAD_SYMBOL_IMPL(library->lib_handle, #name);                                        \
        if(!library->name) {                                                                                           \
            fprintf(stderr, "[WARN] Optional symbol %s not found\n", #name);                                           \
        }                                                                                                              \
    } while(0)

/**
 * loads library for all platforms, exits(1) if there are any errors
 * TODO: fix the flags for windows
 */
static void platform_load_dll(library_api_t *library, const char *file, int flags)
{
#ifdef _WIN32
    // TODO: need to convert const char* to LPCSTR
    lib_handle_t handle = LoadLibrary(file);
    if(!handle) {
        fprintf(stderr, "Library dll %s not found\n", file);
        return;
    }
    library->lib_handle = handle;
#elif __linux__
    lib_handle_t handle = dlopen(file, flags);
    if(!handle) {
        fprintf(stderr, "[CRITICAL] Could not load from library %s\n", dlerror());
        return;
    }
    library->lib_handle = handle;
#else
#error "Platform not Supported"
#endif
}

static int platform_modify_mtime(library_api_t *library)
{
#ifdef _WIN32
#error "NOT IMPLEMENTED"
#elif __linux__
    struct stat st;
    if(stat(library->filename, &st) == 0) {
        library->last_modified = st.st_mtime;
        return 0;
    }
    return 1;
#else
#error "Platform not Supported"
#endif
}

AUTO_RELOAD(check_and_reload_if_needed)
{
    if(!library->auto_reload_enabled) return 0;

#ifdef _WIN32
#error "NOT IMPLEMENTED"
#elif __linux__
    struct stat st;

    struct timespec ts;
    timespec_get(&ts, TIME_UTC);

    // get file attrs
    if(stat(library->filename, &st) == 0) {

        // has library changed ?
        if(st.st_mtime > library->last_modified) {

            // prevent spam reload
            if(ts.tv_sec > st.st_mtime + TIME_BUFFER) {
                fprintf(stdout, "[INFO] Detected game library changes, reloading...\n");
                return load_library(library, library->filename);
            }
        }
    }
#else
#error "Platform not supported"
#endif
    return 1;
}

LOAD_LIBRARY(load_library)
{
    if(library->lib_handle) {
        if(unload_library(library)) {
            return 1;
        }
    }

    platform_load_dll(library, file, RTLD_NOW);

    // insert LOAD_SYMBOL_REQUIRED(fn) here
    assert(0 && "Insert functions here, remove this line afterwards\n");
    LOAD_SYMBOL_OPTIONAL(library, unload_callback);
    LOAD_SYMBOL_OPTIONAL(library, load_callback);

    platform_modify_mtime(library);

    return 0;
}

UNLOAD_LIBRARY(unload_library)
{
    if(library->lib_handle) {
        if(library->unload_callback) {
            if(library->unload_callback() != 0) {
                fprintf(stderr, "[CRITICAL] Could not call library unload callback function\n");
                return 1;
            }
        }
    }
    if(dlclose(library->lib_handle)) {
        return 1;
    }

    return 0;
}
