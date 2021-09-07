#ifndef __SEMPUB_H__
#define __SEMPUB_H__

#include "ipc.h"

union semun {
  int              val;   /* value for SETVAL */
  struct semid_ds *buf;   /* buffer for IPC_STAT, IPC_SET */
  unsigned short * array; /* array for GETALL, SETALL */
  /* Linux specific part: */
  struct seminfo *__buf; /* buffer for IPC_INFO */
};

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

int sem_p(int semid) {
  struct sembuf b = {0, -1, 0};
  int           ret;
  ret = semop(semid, &b, 1);
  if (-1 == ret) {
    ERR_EXIT("semop");
  }

  return ret;
}

int sem_v(int semid) {
  struct sembuf b = {0, 1, 0};
  int           ret;
  ret = semop(semid, &b, 1);
  if (-1 == ret) {
    ERR_EXIT("semop");
  }

  return ret;
}

#endif /* __SEMPUB_H__ */
