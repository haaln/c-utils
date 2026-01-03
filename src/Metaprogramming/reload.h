#ifndef UTIL_RELOAD
#define UTIL_RELOAD

/**
 * Utility for hot reloading, use only on main thread. You're probably doing something wrong if you're reloading on
 * multiple threads. You need to manually add functions in library_api_t, like in the
 * provided example. Furthermore you should also create an 'unload_callback' and 'load_callback' function inside the
 * library if you want to preserve any ultra-temporary state
 */

#include <time.h>

// Delay checking mtime by this amount
#define TIME_BUFFER 1

#define DECLARE_LIBRARY_FUNCTIONS(X)                                                                                   \
    X(int, unload_callback, ())                                                                                        \
    X(int, load_callback, ())

#define TYPEDEF_FUNC(ret, name, args) typedef ret(*name##_t) args;
DECLARE_LIBRARY_FUNCTIONS(TYPEDEF_FUNC)
#undef TYPEDEF_FUNC

typedef struct library_api_t {
    const char *filename;
    time_t last_modified;
    bool auto_reload_enabled;

    void *lib_handle;

#define STRUCT_MEMBER(ret, name, args) name##_t name;
    DECLARE_LIBRARY_FUNCTIONS(STRUCT_MEMBER)
#undef STRUCT_MEMBER
} library_api_t;

extern "C" {

#define LOAD_LIBRARY(name) int name(library_api_t *library, const char *file)
/**
 * The function expects you to add function signatures in library_api_t.
 * You must also add any functions to be added inside the load_library function
 * TODO: find out a way to automatically add functions by introspection of the struct
 */
LOAD_LIBRARY(load_library);

#define UNLOAD_LIBRARY(name) int name(library_api_t *library)
UNLOAD_LIBRARY(unload_library);

#define AUTO_RELOAD(name) int name(library_api_t *library)
AUTO_RELOAD(check_and_reload_if_needed);
}
#endif // UTIL_RELOAD
