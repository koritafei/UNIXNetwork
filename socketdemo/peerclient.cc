/**
 * @file peerclient.cc
 * @author koritafei (koritafei@gmail.com)
 * @brief
 * @version 0.1
 * @date 2021-09-02
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
#include <sys/types.h>
#include <unistd.h>

#include <csignal>

#define MAXLINE 1024

#define ERR_EXIT(m)                                                            \
  do {                                                                         \
    perror(m);                                                                 \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

void handler(int sig) {
  printf("recv a sig = %d\n", sig);
  exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
  int sockfd;

  if (0 > (sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))) {
    ERR_EXIT("socket error");
  }

  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family      = AF_INET;
  servaddr.sin_port        = htons(8899);
  servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

  if (-1 == connect(sockfd,
                    reinterpret_cast<struct sockaddr *>(&servaddr),
                    sizeof(servaddr))) {
    ERR_EXIT("connect error");
  }

  pid_t pid = fork();
  if (-1 == pid) {
    ERR_EXIT("fork error");
  }

  if (0 == pid) {
    // 子进程接收数据
    char buf[MAXLINE];
    for (;;) {
      int ret = read(sockfd, buf, MAXLINE);
      if (0 < ret) {
        printf("%s", buf);
      }
      if (0 == ret) {
        kill(getppid(), SIGUSR1);
        close(sockfd);
        printf("server close\n");
        break;
      }
      if (0 > ret) {
        kill(getppid(), SIGUSR1);
        close(sockfd);
        ERR_EXIT("read error");
      }
      memset(buf, 0, MAXLINE);
    }
  } else {
    signal(SIGUSR1, handler);
    char buf[MAXLINE];
    for (;;) {
      memset(buf, 0, MAXLINE);
      if (NULL != fgets(buf, MAXLINE, stdin)) {
        int ret = write(sockfd, buf, strlen(buf));
        if (ret == 0) {
          printf("peer close\n");
          close(sockfd);
          break;
        }
        if (0 > ret) {
          printf("peer close\n");
          close(sockfd);
          break;
        }
      }
    }
  }

  return 0;
}
