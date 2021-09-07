#ifndef __SYSUTIL_H__
#define __SYSUTIL_H__

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define ERR_EXIT(m)                                                            \
  do {                                                                         \
    perror(m);                                                                 \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

int read_timeout(int fd, unsigned int wait_seconds);
int write_timeout(int fd, unsigned int wait_seconds);
int accpet_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds);

void active_nonblock(int fd);

int connnect_timeout(int                 fd,
                     struct sockaddr_in *addr,
                     unsigned int        wait_seconds);

void deactive_nonblock(int fd);

#endif /* __SYSUTIL_H__ */
