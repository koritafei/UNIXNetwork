/**
 * @file producter.cc
 * @author koritafei (koritafei@gmail.com)
 * @brief
 * @version 0.1
 * @date 2021-09-07
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "ipc.h"
#include "shmfifo.h"

typedef struct stu {
  char name[32];
  int  age;
} STU;

int main(int argc, char **argv) {
  shmfifo_t *fifo = shmfifo_init(1234, sizeof(STU), 3);

  STU s;
  memset(&s, 0, sizeof(STU));
  s.name[0] = 'A';

  for (int i = 0; i < 5; i++) {
    s.age = 20 + i;
    shmfifo_put(fifo, &s);
    s.name[0] = s.name[0] + 1;

    printf("send ok\n");
  }

  return 0;
}
