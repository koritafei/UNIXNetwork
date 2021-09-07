/**
 * @file getrlimit.cc
 * @author koritafei (koritafei@gmail.com)
 * @brief
 * @version 0.1
 * @date 2021-09-03
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>

#define ERR_EXIT(m)                                                            \
  do {                                                                         \
    perror(m);                                                                 \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

int main(int argc, char **argv) {
  struct rlimit rl;
  if (0 > getrlimit(RLIMIT_NOFILE, &rl)) {
    ERR_EXIT("getrlimit error");
  }

  printf("max limit %d\n", (int)rl.rlim_max);
  return 0;
}
