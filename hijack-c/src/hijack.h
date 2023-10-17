#ifndef __HIJACK_C_H__
#define __HIJACK_C_H__

#include <sys/select.h>
#define __USE_GNU
#include <dlfcn.h>
#include <errno.h>

#ifdef USE_HIJACK
#define __use_hijack
#endif

#define HIJACK(n, ret, p...) \
  static ret (*orig_##n)(p); \
  ret n(p)

#define CHECK_HIJACK(n) \
  do {                  \
    hijack_init();      \
    if (!orig_##n) {    \
      errno = ENOSYS;   \
      return -1;        \
    }                   \
  } while (0)

#define FIND_ORIGINAL(n)             \
  do {                               \
    orig_##n = dlsym(RTLD_NEXT, #n); \
  } while (0)

static int inline use_hijack(void) {
#ifdef __use_hijack
  return 1;
#else
  return 0;
#endif
}

static void hijack_init(void);

#endif /* __HIJACK_C_H__*/