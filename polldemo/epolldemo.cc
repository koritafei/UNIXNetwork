/**
 * @file epolldemo.cc
 * @author koritafei (koritafei@gmail.com)
 * @brief
 * @version 0.1
 * @date 2021-09-04
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <arpa/inet.h>
#include <asm-generic/errno-base.h>
#include <asm-generic/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

typedef std::vector<struct epoll_event> EventList;

#define ERR_EXIT(m)                                                            \
  do {                                                                         \
    perror(m);                                                                 \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

void active_nonblock(int fd) {
  int ret;
  int flags = fcntl(fd, F_GETFL);
  if (-1 == flags) {
    ERR_EXIT("fcntl error");
  }

  flags |= O_NONBLOCK;
  ret = fcntl(fd, F_SETFL, flags);

  if (-1 == ret) {
    ERR_EXIT("fcntl error");
  }
}

ssize_t readn(int fd, void *buf, size_t count) {
  size_t  nleft = count;
  ssize_t nread;
  char *  bufp = (char *)(buf);

  while (0 < nleft) {
    if (0 > (nread = read(fd, bufp, nleft))) {
      if (errno == EINTR) {
        continue;
      }
      return -1;
    } else if (nread == 0) {
      return count - nleft;
    }

    bufp += nread;
    nleft -= nread;
  }

  return count;
}

ssize_t writen(int fd, void *buf, size_t count) {
  size_t  nleft = count;
  ssize_t nwrite;
  char *  bufp = (char *)buf;
  while (0 < nleft) {
    if (0 > (nwrite = write(fd, bufp, nleft))) {
      if (errno == EINTR) {
        continue;
      }

      return -1;
    } else if (0 == nwrite) {
      return count - nleft;
    }

    bufp += nwrite;
    nleft -= nwrite;
  }

  return count;
}

ssize_t recv_peek(int sockfd, void *buf, size_t len) {
  for (;;) {
    int ret = recv(sockfd, buf, len, MSG_PEEK);
    if (ret == -1 && errno == EINTR) {
      continue;
    }
    return ret;
  }
}

ssize_t readline(int sockfd, void *buf, ssize_t maxline) {
  int   ret;
  int   nread;
  char *bufp  = (char *)buf;
  int   nleft = maxline;

  for (;;) {
    ret = recv_peek(sockfd, bufp, nleft);
    if (ret < 0) {
      return ret;
    } else if (0 == ret) {
      return ret;
    }

    nread = ret;
    // 查找\n
    for (int i = 0; i < nread; i++) {
      if (bufp[i] == '\n') {
        ret = readn(sockfd, bufp, i + 1);
        if (i + 1 != ret) {
          ERR_EXIT("readn error");
        }
        return ret;
      }
    }

    if (nread > nleft) {
      ERR_EXIT("recv error");
    }

    nleft -= nread;
    ret = readn(sockfd, bufp, nread);
    if (ret != nread) {
      ERR_EXIT("readn error");
    }

    bufp += nread;
  }

  return -1;
}

void signal_sigchld(int sig) {
  while (waitpid(-1, NULL, WNOHANG))
    ;
}

void signal_sigpipe(int sig) {
  printf("recv a sig = %d\n", sig);
}

int main(int argc, char **argv) {
  int ret;

  // 注册信号处理
  signal(SIGCHLD, signal_sigchld);
  signal(SIGPIPE, signal_sigpipe);

  int listenfd;
  // 开启socket
  if (0 > (listenfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))) {
    ERR_EXIT("socket error");
  }

  // bind
  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family      = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port        = htons(8899);

  int on = 1;
  // 设置地址port 断开立刻可用
  if (0 > setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) {
    ERR_EXIT("setsockopt addr error");
  }

  if (0 > setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on))) {
    ERR_EXIT("setsockopt port error");
  }

  if (0 > bind(listenfd,
               reinterpret_cast<struct sockaddr *>(&servaddr),
               sizeof(struct sockaddr))) {
    ERR_EXIT("bind error");
  }

  // listen
  if (0 > listen(listenfd, SOMAXCONN)) {
    ERR_EXIT("listen error");
  }

  // epoll create
  std::vector<int> clients;
  int              epollfd = epoll_create1(EPOLL_CLOEXEC);

  struct epoll_event event;
  event.data.fd = listenfd;
  event.events  = EPOLLIN | EPOLLET;

  epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &event);

  EventList          events(16);
  struct sockaddr_in peeraddr;
  socklen_t          peerlen;
  int                conn;
  int                i;
  int                nready;
  for (;;) {
    nready = epoll_wait(epollfd,
                        &*events.begin(),
                        static_cast<int>(events.size()),
                        -1);

    if (-1 == nready) {
      if (EINTR == errno) {
        continue;
      }
      ERR_EXIT("epoll wait error");
    }
    if (0 == nready) {
      continue;
    }

    if ((size_t)nready == events.size()) {
      events.resize(events.size() * 2);
    }

    for (i = 0; i < nready; i++) {
      if (listenfd == events[i].data.fd) {
        peerlen = sizeof(peeraddr);

        // accept
        conn = accept(listenfd,
                      reinterpret_cast<struct sockaddr *>(&peeraddr),
                      &peerlen);
        if (-1 == conn) {
          ERR_EXIT("accept error");
        }

        printf("ip = %s, port = %d\n",
               inet_ntoa(peeraddr.sin_addr),
               ntohs(peeraddr.sin_port));
        clients.push_back(conn);
        active_nonblock(conn);

        event.data.fd = conn;
        event.events  = EPOLLIN | EPOLLET;
        epoll_ctl(epollfd, EPOLL_CTL_ADD, conn, &event);
      } else if (events[i].events & EPOLLIN) {
        conn = events[i].data.fd;
        if (0 > conn) {
          continue;
        }

        char recvbuf[1024] = {0};
        int  ret           = readline(conn, recvbuf, 1024);

        if (-1 == ret) {
          ERR_EXIT("readline error");
        }

        if (0 == ret) {
          printf("client close\n");
          close(conn);

          event = events[i];
          epoll_ctl(epollfd, EPOLL_CTL_DEL, conn, &event);

          clients.erase(std::remove(clients.begin(), clients.end(), conn),
                        clients.end());
        }

        fputs(recvbuf, stdout);
        write(conn, recvbuf, strlen(recvbuf));
      }
    }
  }

  return 0;
}
