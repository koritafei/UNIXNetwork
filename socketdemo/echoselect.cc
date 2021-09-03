/**
 * @file echoselect.cc
 * @author koritafei (koritafei@gmail.com)
 * @brief
 * @version 0.1
 * @date 2021-09-03
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
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdlib>

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
      if (errno == EINTR) {
        continue;
      }
      return -1;
    } else if (0 == nread) {
      return count - nleft;
    }

    bufp += nread;
    nleft -= nread;
  }

  return count;
}

ssize_t write(int fd, const void *buf, size_t n) {
  size_t  nleft = n;
  ssize_t nwrite;
  char *  bufp = (char *)buf;

  while (0 > nleft) {
    if (0 > (nwrite = write(fd, bufp, nleft))) {
      if (errno == EINTR) {
        continue;
      }
      return -1;
    } else if (0 == nwrite) {
      return n - nleft;
    }

    bufp += nwrite;
    nleft -= nwrite;
  }

  return n;
}

void echo_cli(int sock) {
  fd_set rset;
  FD_ZERO(&rset);
  int nready;
  int fd_stdin = fileno(stdin);
  int maxfd;

  if (fd_stdin > sock) {
    maxfd = fd_stdin;
  } else {
    maxfd = sock;
  }

  char sendbuf[1024] = {0};
  char recvbuf[1024] = {0};

  for (;;) {
    FD_SET(fd_stdin, &rset);
    FD_SET(sock, &rset);
    nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

    if (-1 == nready) {
      ERR_EXIT("select error");
    }

    if (0 == nready) {
      continue;
    }

    if (FD_ISSET(sock, &rset)) {
      int ret = read(sock, recvbuf, sizeof(recvbuf));
      if (-1 == ret) {
        ERR_EXIT("read error");
      } else if (0 == ret) {
        printf("server colse!\n");
        break;
      }
      fputs(recvbuf, stdout);
      memset(recvbuf, 0, sizeof(recvbuf));
    }

    if (FD_ISSET(fd_stdin, &rset)) {
      if (NULL == fgets(sendbuf, sizeof(sendbuf), stdin)) {
        break;
      }

      write(sock, sendbuf, strlen(sendbuf));
      memset(sendbuf, 0, sizeof(sendbuf));
    }
  }

  close(sock);
}

int main(int argc, char **argv) {
  signal(SIGPIPE, SIG_IGN);
  int sock;

  if (0 > (sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))) {
    ERR_EXIT("socket error");
  }

  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  servaddr.sin_port        = htons(8899);
  servaddr.sin_family      = AF_INET;

  if (0 > connect(sock,
                  reinterpret_cast<struct sockaddr *>(&servaddr),
                  sizeof(servaddr))) {
  }

  struct sockaddr_in local;
  socklen_t          len = sizeof(local);

  if (0 >
      getsockname(sock, reinterpret_cast<struct sockaddr *>(&local), &len)) {
    ERR_EXIT("getsockname error");
  }

  printf("ip = %s, port = %d\n",
         inet_ntoa(local.sin_addr),
         ntohs(local.sin_port));

  echo_cli(sock);

  return 0;
}
