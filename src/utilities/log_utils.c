#include "log_utils.h"

#include <esp_log.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

void log_write(
  const char *color,
  const char *level,
  const char *tag,
  const char *func,
  const char *format,
  ...) {
  va_list args;
  va_start(args, format);

  // single lock window so concurrent tasks cannot interleave a partial line
  flockfile(stdout);
  printf("%s%s (%" PRIu32 ") %s", color, level, esp_log_timestamp(), tag);
#if LOG_VERBOSE == 1
  printf(" [%s]", func);
#else
  (void)func;
#endif
  fputs(": ", stdout);
  vprintf(format, args);
  fputs(LOG_ANSI_RESET "\n", stdout);
  funlockfile(stdout);

  va_end(args);
}
