/**
 * @file semgetdemo.cc
 * @author koritafei (koritafei@gmail.com)
 * @brief
 * @version 0.1
 * @date 2021-09-06
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>

union semun {
  int              val;   /* value for SETVAL */
  struct semid_ds *buf;   /* buffer for IPC_STAT, IPC_SET */
  unsigned short * array; /* array for GETALL, SETALL */
  /* Linux specific part: */
  struct seminfo *__buf; /* buffer for IPC_INFO */
};

#define ERR_EXIT(m)                                                            \
  do {                                                                         \
    perror(m);                                                                 \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

int sem_create(key_t key) {
  int semid;
  if (-1 == (semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666))) {
    ERR_EXIT("semget");
  }

  return semid;
}

int sem_open(key_t key) {
  int semid;
  if (-1 == (semid = semget(key, 0, 0))) {
    ERR_EXIT("semget");
  }

  return semid;
}

int sem_setval(int semid, int val) {
  union semun su;
  su.val = val;
  int ret;
  ret = semctl(semid, 0, SETVAL, su);
  if (-1 == ret) {
    ERR_EXIT("semctl");
  }
}

int sem_getval(int semid) {
  int ret;
  ret = semctl(semid, 0, GETVAL, 0);
  if (-1 == ret) {
    ERR_EXIT("semctl");
  }

  return ret;
}

int sem_d(int semid) {
  int ret;
  ret = semctl(semid, 0, IPC_RMID, 0);
  if (-1 == ret) {
    ERR_EXIT("semctl");
  }

  return ret;
}

/*int sem_p(int semid) {
  struct sumbuf b = {0, -1, 0};
  int           ret;
  ret = semop(semid, &b, 1);
  if (-1 == ret) {
    ERR_EXIT("semop");
  }

  return ret;
}

int sem_v(int semid) {
  struct sumbuf b = {0, 1, 0};
  int           ret;
  ret = semop(semid, &b, 1);
  if (-1 == ret) {
    ERR_EXIT("semop");
  }

  return ret;
}*/

int main(int argc, char **argv) {
  int semid;

  semid = sem_create(12345);
  sem_setval(semid, 10);
  printf("get sem = %d\n", sem_getval(semid));
  sem_d(semid);

  return 0;
}
