

#include <arpa/inet.h>
#include <asm-generic/errno-base.h>
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

#define MAXLINE 128

#define ERR_EXIT(m)                                                            \
  do {                                                                         \
    perror(m);                                                                 \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

ssize_t readn(int fd, void *buf, size_t count) {
  size_t  nleft = count;
  ssize_t nread;
  char *  bufp = (char *)buf;
  while (nleft > 0) {
    if (0 > (nread = read(fd, bufp, nleft))) {
      if (EINTR == errno) {
        continue;
      }
      return 1;
    } else if (0 == nread) {
      return count - nleft;
    }

    bufp += nread;
    nleft -= nread;
  }

  return count;
}

ssize_t writen(int fd, const void *buf, size_t n) {
  size_t  nleft = n;
  ssize_t nwrite;
  char *  bufp = (char *)buf;
  while (nleft > 0) {
    if (0 > (nwrite = write(fd, bufp, nleft))) {
      if (EINTR == errno) {
        continue;
      }
      return 1;
    } else if (0 == nwrite) {
      return n - nleft;
    }

    bufp += nwrite;
    nleft -= nwrite;
  }

  return n;
}

void do_service(int conn) {
  char buf[MAXLINE];
  for (;;) {
    memset(buf, 0, MAXLINE);
    int ret = readn(conn, buf, MAXLINE);
    if (0 == ret) {
      printf("client close\n");
      break;
    }
    fputs(buf, stdout);
    writen(conn, buf, ret);
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
