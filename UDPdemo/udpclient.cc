/**
 * @file udpclient.cc
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

void echo_cli(int sock);

int main(int argc, char **argv) {
  int sock;

  if (0 > (sock = socket(PF_INET, SOCK_DGRAM, 0))) {
    ERR_EXIT("udp sock error");
  }

  echo_cli(sock);

  return 0;
}

void echo_cli(int sock) {
  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family      = AF_INET;
  servaddr.sin_port        = htons(8899);
  servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

  char sendbuf[1024] = {0};
  char recvbuf[1024] = {0};

  while (NULL != fgets(sendbuf, sizeof(sendbuf), stdin)) {
    sendto(sock,
           sendbuf,
           sizeof(sendbuf),
           0,
           reinterpret_cast<struct sockaddr *>(&servaddr),
           sizeof(servaddr));
    recvfrom(sock, recvbuf, sizeof(recvbuf), 0, NULL, NULL);
    fputs(recvbuf, stdout);
    memset(sendbuf, 0, sizeof(sendbuf));
    memset(recvbuf, 0, sizeof(recvbuf));
  }
}
