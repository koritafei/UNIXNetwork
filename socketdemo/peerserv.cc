/**
 * @file peerserv.cc
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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdlib>

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
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

  // bind
  if (0 > bind(sockfd,
               reinterpret_cast<struct sockaddr *>(&servaddr),
               sizeof(sockaddr))) {
    ERR_EXIT("bind error");
  }

  int on = 1;
  if (0 > setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) {
    ERR_EXIT("setsockopt error");
  }

  // listen
  if (0 > listen(sockfd, SOMAXCONN)) {
    ERR_EXIT("listen error");
  }

  // accept
  int                conn;
  struct sockaddr_in peeraddr;
  socklen_t          peerlen = sizeof(peeraddr);

  if (0 > (conn = accept(sockfd,
                         reinterpret_cast<struct sockaddr *>(&peeraddr),
                         &peerlen))) {
    ERR_EXIT("accept error");
  }

  printf("connenct ip = %s, port = %d\n",
         inet_ntoa(peeraddr.sin_addr),
         ntohs(peeraddr.sin_port));
  pid_t pid = fork();
  if (-1 == pid) {
    ERR_EXIT("fork error");
  }

  if (0 == pid) {
    signal(SIGUSR1, handler);
    char sendbuf[1024] = {0};
    while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL) {
      int ret = write(conn, sendbuf, strlen(sendbuf));
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
      memset(sendbuf, 0, MAXLINE);
    }
    exit(EXIT_SUCCESS);
  } else {
    // 父进程获取数据
    char buf[MAXLINE];
    for (;;) {
      memset(buf, 0, MAXLINE);
      int ret = read(conn, buf, MAXLINE);
      if (ret == 0) {
        printf("client close\n");
        close(conn);
        close(sockfd);
        kill(pid, SIGUSR1);
        break;
      }
      if (-1 == ret) {
        close(conn);
        close(sockfd);
        kill(pid, SIGUSR1);
        ERR_EXIT("read error");
      }
      fputs(buf, stdout);
    }

    kill(pid, SIGUSR1);
    exit(EXIT_SUCCESS);
  }

  close(conn);
  close(sockfd);

  return 0;
}
