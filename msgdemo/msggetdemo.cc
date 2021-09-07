/**
 * @file msggetdemo.cc
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
  msgid = msgget(1234, 0666 | IPC_CREAT | IPC_EXCL);
  if (-1 == msgid) {
    ERR_EXIT("msgget error");
  }

  printf("msg create success! %d\n", msgid);

  struct msqid_ds buf;
  msgctl(msgid, IPC_STAT, &buf);
  printf("mode=%o\n", buf.msg_perm.mode);
  printf("maxbytes = %d\n", buf.msg_qbytes);

  return 0;
}
