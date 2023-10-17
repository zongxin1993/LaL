#ifndef __LAL_COMMON_LOGGER_H__
#define __LAL_COMMON_LOGGER_H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif
#ifndef NDEBUG
#define DBG(format, ...)                                                 \
  fprintf(stderr,                                                        \
          "[DEBUG] File: " __FILE__ ", Line: %05d: Func: %s -- " format, \
          __LINE__, __func__, ##__VA_ARGS__)
#else
#define DBG(format, ...)
#endif
#define INF(format, ...)                                                \
  fprintf(stderr,                                                       \
          "[INFO] File: " __FILE__ ", Line: %05d: Func: %s -- " format, \
          __LINE__, __func__, ##__VA_ARGS__)
#define ERR(format, ...)                                                 \
  fprintf(stderr,                                                        \
          "[ERROR] File: " __FILE__ ", Line: %05d: Func: %s -- " format, \
          __LINE__, __func__, ##__VA_ARGS__)
#ifdef __cplusplus
}
#endif

#endif  // __LAL_COMMON_LOGGER_H__