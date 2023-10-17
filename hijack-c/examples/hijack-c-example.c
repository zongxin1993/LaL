/*
 * @Author: Zong,xin zongxin@gwm.cn
 * @Date: 2023-05-19 02:37:31
 * @LastEditors: Zong,xin zongxin@gwm.cn
 * @LastEditTime: 2023-05-19 05:12:45
 * @FilePath: /LaL/hijack-c/examples/hijack-c-example.c
 * @Description:
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

int main(int argc, char **argv) {
  int ret = socket(AF_INET, SOCK_STREAM, 0);
  printf("socket returned %d\n", ret);
  return 0;
}