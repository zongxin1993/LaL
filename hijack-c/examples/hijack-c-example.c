#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

int main(int argc, char **argv) {
  int ret = socket(AF_INET, SOCK_STREAM, 0);
  printf("socket returned %d\n", ret);
  return 0;
}