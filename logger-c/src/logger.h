/*
 * @Author: Zong,xin zongxin@gwm.cn
 * @Date: 2023-05-19 01:05:31
 * @LastEditors: Zong,xin zongxin@gwm.cn
 * @LastEditTime: 2023-05-19 01:12:01
 * @FilePath: /LaL/logger-c/src/logger.h
 * @Description:
 */
#ifndef __LOGGER_C_LOGGER_H__
#define __LOGGER_C_LOGGER_H__

#include <stdio.h>

#ifndef NDEBUG
#define DBG(format, ...) fprintf(stderr, "[DEBUG] File: " __FILE__ ", Line: %05d: Func: %s -- " format, \
                                 __LINE__, __func__, ##__VA_ARGS__)
#else
#define DBG(format, ...)
#endif
#define INF(format, ...) fprintf(stderr, "[INFO] File: " __FILE__ ", Line: %05d: Func: %s -- " format, \
                                 __LINE__, __func__, ##__VA_ARGS__)
#define ERR(format, ...) fprintf(stderr, "[ERROR] File: " __FILE__ ", Line: %05d: Func: %s -- " format, \
                                 __LINE__, __func__, ##__VA_ARGS__)

#endif // __LOGGER_C_LOGGER_H__