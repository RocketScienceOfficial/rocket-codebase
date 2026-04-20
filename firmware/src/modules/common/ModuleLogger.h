#pragma once

#define LOG_LEVEL_DEBUG 4
#define LOG_LEVEL_INFO 3
#define LOG_LEVEL_WARN 2
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_OFF 0

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_OFF
#endif

#if LOG_LEVEL_DEBUG <= LOG_LEVEL
#define OBC_DEBUG(...) __module_log("DEBUG", MODULE_NAME, __VA_ARGS__)
#else
#define OBC_DEBUG(...) ((void)0)
#endif

#if LOG_LEVEL_INFO <= LOG_LEVEL
#define OBC_INFO(...) __module_log("INFO", MODULE_NAME, __VA_ARGS__)
#else
#define OBC_INFO(...) ((void)0)
#endif

#if LOG_LEVEL_WARN <= LOG_LEVEL
#define OBC_WARN(...) __module_log("WARN", MODULE_NAME, __VA_ARGS__)
#else
#define OBC_WARN(...) ((void)0)
#endif

#if LOG_LEVEL_ERROR <= LOG_LEVEL
#define OBC_ERROR(...) __module_log("ERROR", MODULE_NAME, __VA_ARGS__)
#else
#define OBC_ERROR(...) ((void)0)
#endif

#if LOG_LEVEL > LOG_LEVEL_OFF
void __module_log(const char *level, const char *module, const char *fmt, ...);
#endif