#include "hijack.h"

#include "../../common/src/logger.h"

int demo() { return 9527; }

HIJACK(socket, int, int domain, int type, int protocol) {
  int ret = -1;
  CHECK_HIJACK(socket);

  if (!use_hijack()) return orig_socket(domain, type, protocol);

  return demo();
}
static void hijack_init(void) {
  static int inited = 0;
  if (inited) return;
  FIND_ORIGINAL(socket);
  inited = 1;
}
