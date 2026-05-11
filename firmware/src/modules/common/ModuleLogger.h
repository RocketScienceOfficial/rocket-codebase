#pragma once

#include <lib/debug/sys_log.h>

#define LOG_LEVEL_DEBUG 4
#define LOG_LEVEL_INFO 3
#define LOG_LEVEL_WARN 2
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_OFF 0

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_OFF
#endif

#if LOG_LEVEL_DEBUG <= LOG_LEVEL
#define LOG_DEBUG(...) __sys_log("DEBUG", MODULE_NAME, __VA_ARGS__)
#else
#define LOG_DEBUG(...) ((void)0)
#endif

#if LOG_LEVEL_INFO <= LOG_LEVEL
#define LOG_INFO(...) __sys_log("INFO", MODULE_NAME, __VA_ARGS__)
#else
#define LOG_INFO(...) ((void)0)
#endif

#if LOG_LEVEL_WARN <= LOG_LEVEL
#define LOG_WARN(...) __sys_log("WARN", MODULE_NAME, __VA_ARGS__)
#else
#define LOG_WARN(...) ((void)0)
#endif

#if LOG_LEVEL_ERROR <= LOG_LEVEL
#define LOG_ERROR(...) __sys_log("ERROR", MODULE_NAME, __VA_ARGS__)
#else
#define LOG_ERROR(...) ((void)0)
#endif