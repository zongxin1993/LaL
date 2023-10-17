/*
 * @Author: Zong,xin zongxin@gwm.cn
 * @Date: 2023-05-19 00:43:54
 * @LastEditors: Zong,xin zongxin@gwm.cn
 * @LastEditTime: 2023-05-19 02:50:42
 * @FilePath: /LaL/common/src/logger.h
 * @Description:
 */
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