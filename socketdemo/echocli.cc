/**
 * @file echocli.cc
 * @author koritafei (koritafei@gmail.com)
 * @brief
 * @version 0.1
 * @date 2021-09-02
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <bits/types/error_t.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define ERR_EXIT(m)                                                            \
  do {                                                                         \
    perror(m);                                                                 \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

int main(int argc, char **argv) {
  int listenfd;

  if (0 > (listenfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))) {
    ERR_EXIT("socket error");
  }

  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));

  servaddr.sin_family      = AF_INET;
  servaddr.sin_port        = ntohs(8899);
  servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

  if (-1 == connect(listenfd,
                    reinterpret_cast<struct sockaddr *>(&servaddr),
                    sizeof(servaddr))) {
    ERR_EXIT("connect error");
  }

  char buf[1024];
  for (;;) {
    memset(buf, 0, 1024);
    if (NULL != fgets(buf, 1024, stdin)) {
      write(listenfd, buf, strlen(buf));
    }

    if (0 != read(listenfd, buf, 1024)) {
      printf("%s", buf);
    } else {
      printf("server close\n");
      break;
    }
  }

  close(listenfd);

  return 0;
}
