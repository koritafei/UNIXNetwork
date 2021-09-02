/**
 * @file echosrv.cc
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

#include <cstdlib>

#define ERR_EXIT(m)                                                            \
  do {                                                                         \
    perror(m);                                                                 \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

void do_service(int conn) {
  char buf[1024];
  for (;;) {
    memset(buf, 0, 1024);
    int ret = read(conn, buf, 1024);
    if (0 == ret) {
      printf("client close\n");
      break;
    }
    fputs(buf, stdout);
    write(conn, buf, ret);
  }
}

int main(int argc, char **argv) {
  int listenfd;
  if (0 > (listenfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))) {
    ERR_EXIT("socket error");
  }

  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family      = AF_INET;
  servaddr.sin_port        = htons(8899);
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

  int on = 1;

  if (0 > setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) {
    ERR_EXIT("setsockopt error");
  }

  if (0 > bind(listenfd,
               reinterpret_cast<struct sockaddr *>(&servaddr),
               sizeof(servaddr))) {
    ERR_EXIT("bind error");
  }

  if (0 > listen(listenfd, SOMAXCONN)) {
    ERR_EXIT("listen error");
  }

  struct sockaddr_in peeraddr;
  socklen_t          addrlen = sizeof(peeraddr);
  int                conn;

  pid_t pid;

  for (;;) {
    if (0 > (conn = accept(listenfd,
                           reinterpret_cast<struct sockaddr *>(&peeraddr),
                           &addrlen))) {
      ERR_EXIT("accept error");
    }

    printf("ip = %s, port = %d connect\n",
           inet_ntoa(peeraddr.sin_addr),
           ntohs(peeraddr.sin_port));
    pid = fork();
    if (-1 == pid) {
      ERR_EXIT("fork error");
    }

    if (0 == pid) {
      close(listenfd);
      do_service(conn);
      exit(EXIT_SUCCESS);
    } else {
      close(conn);
    }
  }

  close(conn);
  close(listenfd);

  return 0;
}
