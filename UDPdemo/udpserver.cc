/**
 * @file udpserver.cc
 * @author koritafei (koritafei@gmail.com)
 * @brief
 * @version 0.1
 * @date 2021-09-05
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
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

void echo_svr(int sock);

int main(int argc, char **argv) {
  int sock;

  if (0 > (sock = socket(PF_INET, SOCK_DGRAM, 0))) {
    ERR_EXIT("udp sock error");
  }

  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family      = AF_INET;
  servaddr.sin_port        = htons(8899);
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (0 > bind(sock,
               reinterpret_cast<struct sockaddr *>(&servaddr),
               sizeof(servaddr))) {
    ERR_EXIT("bind error");
  }

  echo_svr(sock);

  return 0;
}

void echo_svr(int sock) {
  char recvbuf[1024] = {0};

  struct sockaddr_in peeraddr;
  socklen_t          peerlen;
  int                n;

  for (;;) {
    peerlen = sizeof(peeraddr);
    memset(recvbuf, 0, sizeof(recvbuf));

    n = recvfrom(sock,
                 recvbuf,
                 sizeof(recvbuf),
                 0,
                 reinterpret_cast<struct sockaddr *>(&peeraddr),
                 &peerlen);
    if (n == -1) {
      if (errno == EINTR) {
        continue;
      }
      ERR_EXIT("recvfrom");
    } else if (0 < n) {
      fputs(recvbuf, stdout);
      sendto(sock,
             recvbuf,
             n,
             0,
             reinterpret_cast<struct sockaddr *>(&peeraddr),
             peerlen);
    }
  }
  close(sock);
}
