/*
 * @Author: Zong,xin zongxin@gwm.cn
 * @Date: 2023-05-19 01:07:03
 * @LastEditors: Zong,xin zongxin@gwm.cn
 * @LastEditTime: 2023-05-19 01:12:34
 * @FilePath: /LaL/logger-c/examples/logger-c-example.c
 * @Description:
 */
#include "../src/logger.h"

int main(int argc, char **argv)
{
    DBG("Debug logging enabled\n");
    INF("Info logging enabled\n");
    ERR("Error logging enabled\n");
}