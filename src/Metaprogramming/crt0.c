

__attribute__((constructor)) static void on_start() {
  ;
  return;
}

__attribute__((destructor)) static void on_stop() {
  ;
  return;
}

#if (__linux__)
#include <execinfo.h>
#include <signal.h>
#include <sys/resource.h>
#include <unistd.h>

void run_codegen() {
  LOG_DEBUG("Running codegen.\n");
  const char *args[] = {"./code_gen", "0", NULL};
  SDL_Process *res = SDL_CreateProcess(args, true);
  if (SDL_GetProcessOutput(res))
    LOG_WARN("Code generation returned error '%d'.\n", res);
}

/**
 * Recompiles the entire project. Linux version. Uses 'make'.
 *
 * TODO: Check build hash and skip if necessary.
 * TODO: Auto-detect amount of cores.
 */
void self_recompile() {
  LOG_DEBUG("Starting self-recompilation.\n");
  if (SDL_IOFromFile("Makefile", "r")) {
    const char *args[] = {"make game", "", ""};
    SDL_CreateProcess(args, true);
  } else if (SDL_IOFromFile("build.ninja", "r")) {
    const char *args[] = {"ninja", "", ""};
    SDL_CreateProcess(args, true);
  } else {
    LOG_WARN("Could not self-recompile: Unknown build system.\n");
  }
}

void enable_coredump() {
  LOG_DEBUG("Enabling core dump.\n");
  struct rlimit core_limits;
  core_limits.rlim_cur = core_limits.rlim_max = RLIM_INFINITY;
  setrlimit(RLIMIT_CORE, &core_limits);
}

typedef struct crash_info_t {
  volatile i32 sig_nr;
  void *callstack[32];
  int stack_size;
} crash_info_t;

static crash_info_t crash_info;
void symbolicate_crash();
static void crash_signal_handler(int sig, siginfo_t *info, void *ctx) {
  crash_info.sig_nr = sig;
  crash_info.stack_size = backtrace(crash_info.callstack, 32);
  LOG_CRITICAL("Caught fatal signal. Generating crash dump:\n");
  signal(sig, SIG_DFL);
  symbolicate_crash();
  postprocess();
  raise(sig);
}

void symbolicate_crash() {
  if (crash_info.stack_size <= 0)
    return;

  char **symbols =
      backtrace_symbols(crash_info.callstack, crash_info.stack_size);
  if (!symbols)
    return;

  for (int i = 0; i < crash_info.stack_size; ++i) {
    fprintf(stderr, "%d %s\n", i, symbols[i]);
  }

  SDL_free(symbols);
}

void install_crash_handler() {
  LOG_DEBUG("Installing crash handler.\n");
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_SIGINFO | SA_RESTART;
  sa.sa_sigaction = crash_signal_handler;

  int signals[] = {
      SIGSEGV,
      SIGKILL,
      SIGFPE,
      SIGABRT,
  };
  for (int sig : signals) {
    sigaction(sig, &sa, nullptr); // just set to nothing for now.
  }
}
#elif (_WIN32)
#include <dbghelp.h>
#include <windows.h>

LONG WINAPI SehHandler(EXCEPTION_POINTERS *ep) {
  DWORD code = ep->ExceptionRecord->ExceptionCode;

  HANDLE process = GetCurrentProcess();
  HANDLE thread = GetCurrentThread();

  void *stack[64];
  USHORT frames = CaptureStackBackTrace(0, 64, stack, nullptr);

  crash_info.sig_nr = code;
  crash_info.stack_size = frames;
  memcpy(crash_info.callstack, stack, frames * sizeof(void *));

  SymInitialize(process, nullptr, TRUE);

  return EXCEPTION_EXECUTE_HANDLER;
}

void install_windows_handler() { SetUnhandledExceptionFilter(seh_handler); }
#endif
