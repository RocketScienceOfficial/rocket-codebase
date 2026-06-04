#ifndef _SYS_LOG_H_
#define _SYS_LOG_H_

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Log a message with a specified log level and additional information.
 * 
 * @param level The log level (e.g., "INFO", "ERROR").
 * @param additional Additional information to include in the log (e.g., module name).
 * @param fmt The format string for the log message, followed by variable arguments.
 */
void sys_log(const char *level, const char* additional, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif