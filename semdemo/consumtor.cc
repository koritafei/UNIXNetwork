/**
 * @file consumtor.cc
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

  for (int i = 0; i < 5; i++) {
    shmfifo_get(fifo, &s);
    printf("name = %s, age = %d\n", s.name, s.age);
    memset(&s, 0, sizeof(STU));

    printf("recv ok\n");
  }

  return 0;
}
