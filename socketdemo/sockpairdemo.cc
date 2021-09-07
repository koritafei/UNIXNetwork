/**
 * @file sockpairdemo.cc
 * @author koritafei (koritafei@gmail.com)
 * @brief
 * @version 0.1
 * @date 2021-09-06
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define ERR_EXIT(m)                                                            \
  do {                                                                         \
    perror(m);                                                                 \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

int main(void) {
  int sockfds[2];

  if (0 > socketpair(PF_UNIX, SOCK_STREAM, 0, sockfds)) {
    ERR_EXIT("socket pair error");
  }

  pid_t pid;
  pid = fork();

  if (-1 == pid) {
    ERR_EXIT("fork error");
  }

  if (pid > 0) {
    int val = 0;
    close(sockfds[1]);
    for (;;) {
      ++val;
      printf("sending data %d\n", val);
      write(sockfds[0], &val, sizeof(val));
      read(sockfds[0], &val, sizeof(val));
      printf("data recv = %d\n", val);
      sleep(1);
    }

  } else if (0 == pid) {
    int val;
    close(sockfds[0]);
    for (;;) {
      read(sockfds[1], &val, sizeof(val));
      printf("recv data %d\n", val);
      ++val;
      write(sockfds[1], &val, sizeof(val));
      printf("data send = %d\n", val);
    }
  }

  return 0;
}
