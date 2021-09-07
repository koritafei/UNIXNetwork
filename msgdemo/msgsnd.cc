/**
 * @file msgsndrecv.cc
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
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdlib>

#define ERR_EXIT(m)                                                            \
  do {                                                                         \
    perror(m);                                                                 \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

int main(int argc, char **argv) {
  int msgid;
  msgid    = msgget(1234, 0);
  int len  = atoi(argv[1]);
  int type = atoi(argv[2]);
  if (-1 == msgid) {
    ERR_EXIT("msgget error");
  }

  struct msgbuf *ptr;

  ptr        = (struct msgbuf *)malloc(sizeof(long) + len);
  ptr->mtype = type;

  if (0 > msgsnd(msgid, ptr, len, 0)) {
    ERR_EXIT("msgsnd error");
  }

  return 0;
}