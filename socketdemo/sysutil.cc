#include "sysutil.h"

/**
 * @brief 读超时操作函数，不含读操作
 *
 * @param fd 文件描述符
 * @param wait_seconds 等待超时秒数
 * @return int 成功返回0，失败返回-1， 超时返回-1，并设置errno=ETIMEDOUT
 */
int read_timeout(int fd, unsigned int wait_seconds) {
  int ret;
  if (0 < wait_seconds) {
    fd_set         read_fdset;
    struct timeval timeout;

    FD_ZERO(&read_fdset);
    FD_SET(fd, &read_fdset);
    timeout.tv_sec  = wait_seconds;
    timeout.tv_usec = 0;

    do {
      ret = select(fd + 1, &read_fdset, NULL, NULL, &timeout);
    } while (ret < 0 && errno == EINTR);
  }

  if (0 == ret) {
    ret   = -1;
    errno = ETIMEDOUT;
  } else if (-1 == ret) {
    ret = 0;
  }

  return ret;
}

/**
 * @brief 写超时检测函数，不含写操作
 *
 * @param fd 文件描述符
 * @param wait_seconds 超时时间
 * @return int 成功返回0，失败返回-1， 超时返回-1， errno = ETIMEDOUT
 */
int write_timeout(int fd, unsigned int wait_seconds) {
  int ret = 0;
  if (0 < wait_seconds) {
    fd_set write_set;

    FD_ZERO(&write_set);
    FD_SET(fd, &write_set);

    struct timeval timeout;
    timeout.tv_sec  = wait_seconds;
    timeout.tv_usec = 0;

    do {
      ret = select(fd + 1, NULL, &write_set, NULL, &timeout);
    } while (0 < ret && errno == EINTR);
  }

  if (ret < 0) {
    ret   = -1;
    errno = ETIMEDOUT;
  } else if (ret == -1) {
    ret = 0;
  }

  return ret;
}

/**
 * @brief 带超时的accept
 *
 * @param fd 文件描述符
 * @param addr 输出参数，返回对方地址
 * @param wait_seconds 等待超时秒数，如果为0则为正常模式
 * @return int 成功(未超时)返回已连接的套接字，超时返回-1，errno=ETIMEDOUT
 */
int accpet_timeout(int                 fd,
                   struct sockaddr_in *addr,
                   unsigned int        wait_seconds) {
  int       ret     = 0;
  socklen_t addrlen = sizeof(struct sockaddr_in);
  if (wait_seconds > 0) {
    fd_set         accept_set;
    struct timeval timeout;
    FD_ZERO(&accept_set);
    FD_SET(fd, &accept_set);
    timeout.tv_sec  = wait_seconds;
    timeout.tv_usec = 0;

    do {
    } while (ret < 0 && errno == EINTR);

    if (ret == 0) {
      errno = ETIMEDOUT;
      return -1;
    } else if (ret == -1) {
      return -1;
    }
  }

  if (NULL != addr) {
    ret = accept(fd, reinterpret_cast<struct sockaddr *>(addr), &addrlen);
  } else {
    ret = accept(fd, NULL, NULL);
  }

  if (-1 == ret) {
    ERR_EXIT("accept error");
  }

  return ret;
}

/**
 * @brief 设置I/O为阻塞模式
 *
 * @param fd 文件描述符
 * @return void
 */
void active_nonblock(int fd) {
  int ret;
  int flags = fcntl(fd, F_GETFL);

  if (-1 == flags) {
    ERR_EXIT("fcntl error");
  }

  flags &= ~O_NONBLOCK;
  ret = fcntl(fd, F_SETFL, flags);
  if (-1 == flags) {
    ERR_EXIT("fcntl error");
  }
}

/**
 * @brief 设置I/O为非阻塞模式
 *
 * @param fd 文件描述符
 * @return void
 */
void deactive_nonblock(int fd) {
  int ret;
  int flags = fcntl(fd, F_GETFL);

  if (-1 == flags) {
    ERR_EXIT("fcntl error");
  }

  flags &= O_NONBLOCK;
  ret = fcntl(fd, F_SETFL, flags);
  if (-1 == flags) {
    ERR_EXIT("fcntl error");
  }
}

/**
 * @brief connect 超时函数
 *
 * @param fd 文件描述符
 * @param addr 要连接对方地址
 * @param wait_seconds 超时等待时间
 * @return int 成功(未超时)返回0， 失败返回-1，超时返回-1，errno=ETIMEDOUT
 */
int connnect_timeout(int                 fd,
                     struct sockaddr_in *addr,
                     unsigned int        wait_seconds) {
  int       ret;
  socklen_t addrlen = sizeof(struct sockaddr_in);

  if (wait_seconds > 0) {
    active_nonblock(fd);
  }

  ret = connect(fd, reinterpret_cast<struct sockaddr *>(addr), addrlen);
  if (ret < 0 && errno == EINPROGRESS) {
    fd_set         connect_set;
    struct timeval timeout;
    FD_ZERO(&connect_set);
    FD_SET(fd, &connect_set);

    // 连接建立即可写
    do {
      ret = select(fd + 1, NULL, &connect_set, NULL, &timeout);
    } while (ret < 0 && errno == EINTR);

    if (0 == ret) {
      ret   = -1;
      errno = ETIMEDOUT;
    } else if (ret < 0) {
      return -1;
    } else if (1 == ret) {
      // ret 返回1的两种情况：
      // 1. 连接建立成功
      // 2.
      // 套接字产生错误，此时错误信息不会保留在errno变量中，需要通过getsockopt来获取
      int       err;
      socklen_t socklen = sizeof(err);
      int sockoptret    = getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &socklen);

      if (-1 == sockoptret) {
        return -1;
      }

      if (0 == err) {
        ret = 0;
      } else {
        errno = err;
        ret   = -1;
      }
    }
  }

  if (wait_seconds > 0) {
    deactive_nonblock(fd);
  }

  return ret;
}