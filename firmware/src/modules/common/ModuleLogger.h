#pragma once

#define OBC_LOG_LEVEL_DEBUG 4
#define OBC_LOG_LEVEL_INFO 3
#define OBC_LOG_LEVEL_WARN 2
#define OBC_LOG_LEVEL_ERROR 1
#define OBC_LOG_LEVEL_OFF 0

#ifndef OBC_LOG_LEVEL
#define OBC_LOG_LEVEL OBC_LOG_LEVEL_OFF
#endif

#if OBC_LOG_LEVEL_DEBUG <= OBC_LOG_LEVEL
#define OBC_DEBUG(...) __module_log("DEBUG", MODULE_NAME, __VA_ARGS__)
#else
#define OBC_DEBUG(...) ((void)0)
#endif

#if OBC_LOG_LEVEL_INFO <= OBC_LOG_LEVEL
#define OBC_INFO(...) __module_log("INFO", MODULE_NAME, __VA_ARGS__)
#else
#define OBC_INFO(...) ((void)0)
#endif

#if OBC_LOG_LEVEL_WARN <= OBC_LOG_LEVEL
#define OBC_WARN(...) __module_log("WARN", MODULE_NAME, __VA_ARGS__)
#else
#define OBC_WARN(...) ((void)0)
#endif

#if OBC_LOG_LEVEL_ERROR <= OBC_LOG_LEVEL
#define OBC_ERROR(...) __module_log("ERROR", MODULE_NAME, __VA_ARGS__)
#else
#define OBC_ERROR(...) ((void)0)
#endif

#if OBC_LOG_LEVEL > OBC_LOG_LEVEL_OFF
void __module_log(const char *level, const char *module, const char *fmt, ...);
#endif