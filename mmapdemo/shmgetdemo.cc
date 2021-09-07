/**
 * @file shmgetdemo.cc
 * @author koritafei (koritafei@gmail.com)
 * @brief
 * @version 0.1
 * @date 2021-09-06
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define ERR_EXIT(m)                                                            \
  do {                                                                         \
    perror(m);                                                                 \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

typedef struct stu {
  char name[32];
  int  age;
} STU;

int main(int argc, char **argv) {
  int shmid;
  shmid = shmget(1234, sizeof(STU), IPC_CREAT | 0666);
  if (-1 == shmid) {
    ERR_EXIT("shmget");
  }

  return 0;
}
