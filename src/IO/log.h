#ifndef UTIL_LOG
#define UTIL_LOG

extern "C" {
void __log_trace(const char *format, ...);
void __log_debug(const char *format, ...);
void __log_info(const char *format, ...);
void __log_warn(const char *format, ...);
void __log_error(const char *format, ...);
void __log_critical(const char *format, ...);

} // extern "C"

// Define a LOG_LEVEL inside a header somewhere in the project
#ifndef NDEBUG
#if (LOG_LEVEL == 3)
#define LOG_TRACE(fmt, ...) __log_trace("[%s] " fmt, __FUNCTION__, ##__VA_ARGS__)
#define LOG_DEBUG(...)      __log_debug(__VA_ARGS__)
#elif (LOG_LEVEL == 2)
#define LOG_TRACE(...)
#define LOG_DEBUG(...) __log_debug(__VA_ARGS__)
#else
#define LOG_TRACE(...)
#define LOG_DEBUG(...)
#endif
#define LOG_INFO(...)     __log_info(__VA_ARGS__)
#define LOG_WARN(...)     __log_warn(__VA_ARGS__)
#define LOG_ERROR(...)    __log_error(__VA_ARGS__)
#define LOG_CRITICAL(...) __log_critical(__VA_ARGS__)
#else
#define LOG_TRACE(...)
#define LOG_DEBUG(...)
#define LOG_WARN(...)
#define LOG_ERROR(...)
#define LOG_CRITICAL(...)
#endif

#endif // UTIL_LOG
