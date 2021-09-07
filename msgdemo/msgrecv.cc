/**
 * @file msgrecv.cc
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
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define ERR_EXIT(m)                                                            \
  do {                                                                         \
    perror(m);                                                                 \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

int main(int argc, char **argv) {
  int flag = 0;
  int type = 0;
  int opt;

  while (-1 != (opt = getopt(argc, argv, "nt:"))) {
    if ('?' == opt) {
      ERR_EXIT("getopt");
    }

    switch (opt) {
      case 'n':
        printf("AAAA\n");
        flag |= IPC_NOWAIT;
        break;
      case 't':
        printf("BBBB\n");
        int n = atoi(optarg);
        printf("n=%d\n", n);
        type = atoi(optarg);
        break;
    }
  }

  int msgid;
  msgid = msgget(1234, 0);
  if (-1 == msgid) {
    ERR_EXIT("msgget error");
  }

  struct msgbuf *ptr;

  ptr        = (struct msgbuf *)malloc(sizeof(long) + 1024);
  ptr->mtype = type;

  int nrecv;

  if (0 > (nrecv = msgrcv(msgid, ptr, 1024, type, flag))) {
    ERR_EXIT("msgsnd error");
  }

  printf("read %d bytes, type=%ld\n", nrecv, ptr->mtype);

  return 0;
}