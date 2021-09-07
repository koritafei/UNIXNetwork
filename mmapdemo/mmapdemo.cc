/**
 * @file mmapdemo.cc
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
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define ERR_EXIT(m)                                                            \
  do {                                                                         \
    perror(m);                                                                 \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

typedef struct stu {
  char name[4];
  int  age;
} STU;

int main(int argc, char **argv) {
  int fd;
  if (-1 == (fd = open(argv[1], O_CREAT | O_RDWR | O_TRUNC, 0666))) {
    ERR_EXIT("open");
  }

  lseek(fd, sizeof(STU) * 5 - 1, SEEK_SET);
  write(fd, "", 1);

  STU *p;
  p = (STU *)
      mmap(NULL, sizeof(STU) * 5, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (NULL == p) {
    ERR_EXIT("mmap");
  }

  char ch = 'a';
  int  i;
  for (i = 0; i < 5; i++) {
    memcpy((p + i)->name, &ch, sizeof(ch));
    (p + i)->age = 20 + i;
    ch++;
  }

  printf("initialize over\n");
  munmap(p, sizeof(STU) * 5);
  printf("...exit\n");

  return 0;
}
