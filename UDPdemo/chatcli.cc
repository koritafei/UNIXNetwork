/**
 * @file chatcli.cc
 * @author koritafei (koritafei@gmail.com)
 * @brief
 * @version 0.1
 * @date 2021-09-05
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdlib>

#include "chatpub.h"

#define ERR_EXIT(m)                                                            \
  do {                                                                         \
    perror(m);                                                                 \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

char      username[16];
USER_LIST client_list;

bool sendmsgto(int sock, char *name, char *msg) {
  if (0 == strcmp(name, username)) {
    printf("cann't send msg to self\n");
    return false;
  }

  if (0 == client_list.count(name)) {
    printf("user %d has not login server\n", name);
    return false;
  }

  USER_INFO user = client_list[name];

  MESSAGE m;
  memset(&m, 0, sizeof(m));
  m.cmd = htonl(C2C_CHAT);

  CHAT_MSG cm;
  strcpy(cm.user_name, username);
  strcpy(cm.msg, msg);

  memcpy(m.body, &cm, sizeof(cm));

  struct sockaddr_in peeraddr;
  memset(&peeraddr, 0, sizeof(peeraddr));
  peeraddr.sin_family      = AF_INET;
  peeraddr.sin_addr.s_addr = htonl(user.ip);
  peeraddr.sin_port        = htons(user.port);

  in_addr tmp;
  tmp.s_addr = user.ip;

  printf("sending message [%s] to user [%s] <-> %s:%d\n",
         msg,
         name,
         inet_ntoa(tmp),
         ntohs(user.ip));

  sendto(sock,
         (const char *)&m,
         sizeof(m),
         0,
         reinterpret_cast<struct sockaddr *>(&peeraddr),
         sizeof(peeraddr));

  return true;
}

void parse_cmd(char *cmdline, int sock, struct sockaddr_in *servaddr) {
  char  cmd[10] = {0};
  char *p;

  p = strchr(cmdline, ' ');
  if (NULL != p) {
    *p = '\0';
  }
  strcpy(cmd, cmdline);

  if (0 == strcmp(cmd, "exit")) {
    MESSAGE msg;
    memset(&msg, 0, sizeof(msg));
    msg.cmd = htonl(C2S_LOGOUT);
    strcpy(msg.body, username);

    if (0 > sendto(sock,
                   &msg,
                   sizeof(msg),
                   0,
                   reinterpret_cast<struct sockaddr *>(servaddr),
                   sizeof(struct sockaddr))) {
      ERR_EXIT("sendto error");
    }
    printf("user %s has logout servser\n", username);
    exit(EXIT_SUCCESS);
  } else if (0 == strcmp(cmd, "send")) {
    char peername[16] = {0};
    char msg[MSG_LEN] = {0};
    while (*p++ == ' ') {
    }
    char *p2;
    p2 = strchr(p, ' ');
    if (NULL == p2) {
      printf("\n Bad Commands:\n");
      printf(" send username msg\n");
      printf(" list\n");
      printf(" exit\n");
      printf("\n");
    }
    *p2 = '\0';
    strcpy(msg, p2);
    sendmsgto(sock, peername, msg);
  } else if (0 == strcmp(cmd, "list")) {
    printf("cmd %s\n", cmd);

    MESSAGE msg;
    memset(&msg, 0, sizeof(msg));
    msg.cmd = htonl(C2S_ONLINE_USER);

    if (0 > sendto(sock,
                   &msg,
                   sizeof(msg),
                   0,
                   reinterpret_cast<struct sockaddr *>(servaddr),
                   sizeof(struct sockaddr))) {
      ERR_EXIT("sendto error");
    }
  } else {
    printf("\n Bad Commands:\n");
    printf(" send username msg\n");
    printf(" list\n");
    printf(" exit\n");
    printf("\n");
  }
}

void do_getlist(int sock) {
  int count;
  recvfrom(sock, &count, sizeof(count), 0, NULL, NULL);
  printf("has %s users logined server\n", count);
  client_list.clear();
  int n = count;
  for (int i = 0; i < n; i++) {
    USER_INFO user;
    recvfrom(sock, &user, sizeof(user), 0, NULL, NULL);
    in_addr tmp;
    tmp.s_addr = user.ip;
    printf("%s <-> %s:%d\n", user.username, inet_ntoa(tmp), ntohl(user.ip));
    client_list[user.username] = user;
  }
}

void do_someone_login(MESSAGE &msg) {
  USER_INFO *user = reinterpret_cast<USER_INFO *>(&msg.body);

  if (0 != client_list.count(msg.body)) {
    printf("user %s has login server\n", user->username);
    return;
  }

  in_addr tmp;
  tmp.s_addr = user->ip;
  printf("%s <-> %s:%d has login server\n",
         user->username,
         inet_ntoa(tmp),
         ntohl(user->port));
}

void do_someone_logout(MESSAGE &msg) {
  client_list.erase(msg.body);
  printf("user %s has logout server\n", msg.body);
}

void do_chat(const MESSAGE &msg) {
  CHAT_MSG *cm = (CHAT_MSG *)msg.body;
  printf("recv a msg [%s] from [%s]\n", cm->msg, cm->user_name);
}

void echo_cli(int sock, struct sockaddr_in *servaddr) {
  struct sockaddr_in peeraddr;
  socklen_t          peerlen = sizeof(peeraddr);

  MESSAGE msg;
  for (;;) {
    memset(username, 0, sizeof(username));
    printf("please input your name:");
    fflush(stdout);
    scanf("%s", &username);

    memset(&msg, 0, sizeof(msg));
    msg.cmd = htonl(C2S_LOGIN);
    memcpy(&msg.body, &username, sizeof(username));

    sendto(sock,
           &msg,
           sizeof(msg),
           0,
           reinterpret_cast<struct sockaddr *>(servaddr),
           sizeof(struct sockaddr));

    memset(&msg, 0, sizeof(msg));

    recvfrom(sock, &msg, sizeof(msg), 0, NULL, NULL);

    int cmd = ntohl(msg.cmd);
    if (cmd == S2C_ALREADY_LOGINED) {
      printf("user %s has logined server, please use another name\n", username);
      break;
    } else if (cmd == S2C_LOGIN_OK) {
      printf("user %s has login server\n", username);
      break;
    }
  }

  // 接收在线人数
  int count;

  recvfrom(sock, &count, sizeof(count), 0, NULL, NULL);
  int n = count;
  printf("Total %d user online\n", n);

  for (int i = 0; i < n; i++) {
    USER_INFO user;
    recvfrom(sock, &user, sizeof(user), 0, NULL, NULL);
    client_list[user.username] = user;
    in_addr tmp;

    tmp.s_addr = user.ip;
    printf("%d %s  <-> %s:%d\n", i, username, inet_ntoa(tmp), ntohs(user.port));
  }

  printf("\n Commands:\n");
  printf(" send username msg\n");
  printf(" list\n");
  printf(" exit\n");
  printf("\n");

  fd_set rset;
  FD_ZERO(&rset);
  for (;;) {
    FD_SET(STDIN_FILENO, &rset);
    FD_SET(sock, &rset);
    int nready = select(sock + 1, &rset, NULL, NULL, NULL);
    if (-1 == nready) {
      ERR_EXIT("select error");
    }

    if (0 == nready) continue;

    if (FD_ISSET(sock, &rset)) {
      peerlen = sizeof(peeraddr);
      memset(&msg, 0, sizeof(msg));
      recvfrom(sock,
               &msg,
               sizeof(msg),
               0,
               reinterpret_cast<struct sockaddr *>(&peeraddr),
               &peerlen);

      int cmd = ntohl(msg.cmd);

      switch (cmd) {
        case S2C_SOMEONE_LOGIN:
          do_someone_login(msg);
          break;
        case S2C_SOMEONE_LOGOUT:
          do_someone_logout(msg);
          break;
        case S2C_ONLINE_USER:
          do_getlist(sock);
          break;
        case C2C_CHAT:
          do_chat(msg);
          break;

        default:
          break;
      }
    }

    if (FD_ISSET(STDIN_FILENO, &rset)) {
      char cmdline[100] = {0};
      if (NULL == fgets(cmdline, sizeof(cmdline), stdin)) {
        break;
      }

      if (cmdline[0] == '\n') {
        continue;
      }

      cmdline[strlen(cmdline) - 1] = '\0';

      parse_cmd(cmdline, sock, servaddr);
    }
  }

  memset(&msg, 0, sizeof(msg));
  msg.cmd = htonl(C2S_LOGOUT);
  strcpy(msg.body, username);

  sendto(sock,
         (const char *)&msg,
         sizeof(msg),
         0,
         reinterpret_cast<struct sockaddr *>(servaddr),
         sizeof(servaddr));

  close(sock);
}

int main(int argc, char **argv) {
  // socket
  int sock;
  if (0 > (sock = socket(PF_INET, SOCK_DGRAM, 0))) {
    ERR_EXIT("socket error");
  }

  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family      = AF_INET;
  servaddr.sin_port        = htons(8899);
  servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

  echo_cli(sock, &servaddr);

  return 0;
}
