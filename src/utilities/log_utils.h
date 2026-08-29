#pragma once

#define LOG_VERBOSE         0             /* Config: enable/disable logging with function name */

#define LOG_LEVEL_NOTICE    1             /* Config: enable/disable notice logs */
#define LOG_LEVEL_DEBUG     0             /* Config: enable/disable debug logs */
#define LOG_LEVEL_INFO      1             /* Config: enable/disable info logs */
#define LOG_LEVEL_WARNING   1             /* Config: enable/disable warning logs */
#define LOG_LEVEL_ERROR     1             /* Config: enable/disable error logs */
#define LOG_LEVEL_FATAL     1             /* Config: enable/disable fatal logs */

#define LOG_ANSI_RESET      "\033[0m"     /* Format: Reset */

#define LOG_ANSI_STD_BK     "\033[0;30m"  /* Color: Black */
#define LOG_ANSI_STD_RD     "\033[0;31m"  /* Color: Red */
#define LOG_ANSI_STD_GN     "\033[0;32m"  /* Color: Green */
#define LOG_ANSI_STD_YL     "\033[0;33m"  /* Color: Yellow */
#define LOG_ANSI_STD_BL     "\033[0;34m"  /* Color: Blue */
#define LOG_ANSI_STD_MG     "\033[0;35m"  /* Color: Magenta */
#define LOG_ANSI_STD_CY     "\033[0;36m"  /* Color: Cyan */
#define LOG_ANSI_STD_WH     "\033[0;37m"  /* Color: White */

#define LOG_ANSI_BRT_BK     "\033[0;90m"  /* Color: Bright Black */
#define LOG_ANSI_BRT_RD     "\033[0;91m"  /* Color: Bright Red */
#define LOG_ANSI_BRT_GN     "\033[0;92m"  /* Color: Bright Green */
#define LOG_ANSI_BRT_YL     "\033[0;93m"  /* Color: Bright Yellow */
#define LOG_ANSI_BRT_BL     "\033[0;94m"  /* Color: Bright Blue */
#define LOG_ANSI_BRT_MG     "\033[0;95m"  /* Color: Bright Magenta */
#define LOG_ANSI_BRT_CY     "\033[0;96m"  /* Color: Bright Cyan */
#define LOG_ANSI_BRT_WH     "\033[0;97m"  /* Color: Bright White */

#define LOG_ANSI_BKG_STD_BK "\033[0;40m"  /* Background: Black */
#define LOG_ANSI_BKG_STD_RD "\033[0;41m"  /* Background: Red */
#define LOG_ANSI_BKG_STD_GN "\033[0;42m"  /* Background: Green */
#define LOG_ANSI_BKG_STD_YL "\033[0;43m"  /* Background: Yellow */
#define LOG_ANSI_BKG_STD_BL "\033[0;44m"  /* Background: Blue */
#define LOG_ANSI_BKG_STD_MG "\033[0;45m"  /* Background: Magenta */
#define LOG_ANSI_BKG_STD_CY "\033[0;46m"  /* Background: Cyan */
#define LOG_ANSI_BKG_STD_WH "\033[0;47m"  /* Background: White */

#define LOG_ANSI_BKG_BRT_BK "\033[0;100m" /* Background: Bright Black */
#define LOG_ANSI_BKG_BRT_RD "\033[0;101m" /* Background: Bright Red */
#define LOG_ANSI_BKG_BRT_GN "\033[0;102m" /* Background: Bright Green */
#define LOG_ANSI_BKG_BRT_YL "\033[0;103m" /* Background: Bright Yellow */
#define LOG_ANSI_BKG_BRT_BL "\033[0;104m" /* Background: Bright Blue */
#define LOG_ANSI_BKG_BRT_MG "\033[0;105m" /* Background: Bright Magenta */
#define LOG_ANSI_BKG_BRT_CY "\033[0;106m" /* Background: Bright Cyan */
#define LOG_ANSI_BKG_BRT_WH "\033[0;107m" /* Background: Bright White */

/**
 * @brief Writes a formatted log message with color, level, tag, and function information
 *
 * @param color   ANSI color code
 * @param level   Log level
 * @param tag     Tag
 * @param func    Function name
 * @param format  printf-style format string
 * @param ...     Arguments for format string
 */
__attribute__((format(printf, 5, 6))) void log_write(
  const char *color,
  const char *level,
  const char *tag,
  const char *func,
  const char *format,
  ...);

#if LOG_VERBOSE == 1
#define LOG_FUNC __func__
#else
#define LOG_FUNC NULL
#endif

#define LOG_EMIT(color, level, tag, format, ...) \
  log_write(color, level, tag, LOG_FUNC, format __VA_OPT__(, ) __VA_ARGS__)

#define LOG_DISCARD(tag, format, ...)                                      \
  do {                                                                     \
    if (0) {                                                               \
      log_write("", "", tag, LOG_FUNC, format __VA_OPT__(, ) __VA_ARGS__); \
    }                                                                      \
  } while (0)

#if LOG_LEVEL_NOTICE == 1
/* NOTICE log */
#define LOGN(tag, format, ...) LOG_EMIT(LOG_ANSI_STD_BL, "N", tag, format __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOGN(tag, format, ...) LOG_DISCARD(tag, format __VA_OPT__(, ) __VA_ARGS__)
#endif

#if LOG_LEVEL_DEBUG == 1
/* DEBUG log */
#define LOGD(tag, format, ...) LOG_EMIT(LOG_ANSI_STD_CY, "D", tag, format __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOGD(tag, format, ...) LOG_DISCARD(tag, format __VA_OPT__(, ) __VA_ARGS__)
#endif

#if LOG_LEVEL_INFO == 1
/* INFO log */
#define LOGI(tag, format, ...) LOG_EMIT(LOG_ANSI_STD_GN, "I", tag, format __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOGI(tag, format, ...) LOG_DISCARD(tag, format __VA_OPT__(, ) __VA_ARGS__)
#endif

#if LOG_LEVEL_WARNING == 1
/* WARNING log */
#define LOGW(tag, format, ...) LOG_EMIT(LOG_ANSI_STD_YL, "W", tag, format __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOGW(tag, format, ...) LOG_DISCARD(tag, format __VA_OPT__(, ) __VA_ARGS__)
#endif

#if LOG_LEVEL_ERROR == 1
/* ERROR log */
#define LOGE(tag, format, ...) LOG_EMIT(LOG_ANSI_STD_RD, "E", tag, format __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOGE(tag, format, ...) LOG_DISCARD(tag, format __VA_OPT__(, ) __VA_ARGS__)
#endif

#if LOG_LEVEL_FATAL == 1
/* FATAL log */
#define LOGF(tag, format, ...) LOG_EMIT(LOG_ANSI_STD_MG, "F", tag, format __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOGF(tag, format, ...) LOG_DISCARD(tag, format __VA_OPT__(, ) __VA_ARGS__)
#endif
