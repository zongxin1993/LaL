#ifndef __LAL_SOCK_UTILS_HH__
#define __LAL_SOCK_UTILS_HH__

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdio>

#include "../../common/src/logger.h"

typedef enum _sock_conn_status {
  disconnect = -1,
  normal = 0,
  readable = 1
} sock_conn_status_t;

#endif  //__LAL_SOCK_UTILS_HH__