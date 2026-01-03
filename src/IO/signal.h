#ifndef SIGNAL_H_
#define SIGNAL_H_

#include <csignal>
#ifndef NDEBUG

typedef void(AbrtHandler)(int);

// gather memory dumps, stack traces, etc...
static inline void register_abrt(AbrtHandler *fn)
{
    signal(SIGABRT, fn);
}

#define OK
#endif

#else
#error "multiple include"
#endif // SIGNAL_H_
