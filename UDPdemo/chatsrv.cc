/**
 * @file chatsrv.cc
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
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "chatpub.h"

#define ERR_EXIT(m)                                                            \
  do {                                                                         \
    perror(m);                                                                 \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

USER_LIST client_list;

void chat_srv(int sock);
void do_login(MESSAGE &msg, int sock, struct sockaddr_in *cliaddr);
void do_logout(MESSAGE &msg, int sock, struct sockaddr_in *cliaddr);
void do_sendlist(int sock, struct sockaddr_in *cliaddr) {
  MESSAGE msg;
  memset(&msg, 0, sizeof(msg));

  // 发送在线数据包
  msg.cmd = ntohl(S2C_ONLINE_USER);
  sendto(sock,
         &msg,
         sizeof(msg),
         0,
         reinterpret_cast<struct sockaddr *>(cliaddr),
         sizeof(cliaddr));

  int count = client_list.size();
  // 发送在线人数
  sendto(sock,
         &count,
         sizeof(count),
         0,
         reinterpret_cast<struct sockaddr *>(cliaddr),
         sizeof(cliaddr));

  for (auto it : client_list) {
    sendto(sock,
           &(it.second),
           sizeof(USER_INFO),
           0,
           reinterpret_cast<struct sockaddr *>(cliaddr),
           sizeof(cliaddr));
  }
}

int main(int argc, char **argv) {
  int sock;
  if (0 > (sock = socket(PF_INET, SOCK_DGRAM, 0))) {
    ERR_EXIT("socket error");
  }

  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family      = AF_INET;
  servaddr.sin_port        = htons(8899);
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (0 > bind(sock,
               reinterpret_cast<struct sockaddr *>(&servaddr),
               sizeof(servaddr))) {
    ERR_EXIT("bind error");
  }

  chat_srv(sock);

  return 0;
}

void chat_srv(int sock) {
  struct sockaddr_in cliaddr;
  socklen_t          clilen;
  int                n;
  MESSAGE            msg;
  for (;;) {
    memset(&msg, 0, sizeof(msg));
    clilen = sizeof(cliaddr);

    n = recvfrom(sock,
                 &msg,
                 sizeof(msg),
                 0,
                 reinterpret_cast<struct sockaddr *>(&cliaddr),
                 &clilen);

    if (0 > n) {
      if (errno == EINTR) {
        continue;
      }
      ERR_EXIT("recv from error");
    }

    int cmd = ntohl(msg.cmd);
    printf("recv cmd = %d\n", cmd);

    switch (cmd) {
      case C2S_LOGIN:
        do_login(msg, sock, &cliaddr);
        break;
      case C2S_LOGOUT:
        do_logout(msg, sock, &cliaddr);
        break;
      case C2S_ONLINE_USER:
        do_sendlist(sock, &cliaddr);
        break;
      default:
        break;
    }
  }
}

void do_login(MESSAGE &msg, int sock, struct sockaddr_in *cliaddr) {
  USER_INFO user;
  strcpy(user.username, msg.body);
  user.ip   = cliaddr->sin_addr.s_addr;
  user.port = cliaddr->sin_port;

  // 查找用户
  // 未找到
  if (0 == client_list.count(msg.body)) {
    printf("has a user login: %s <-> %s:%d \n",
           msg.body,
           inet_ntoa(cliaddr->sin_addr),
           ntohs(cliaddr->sin_port));

    printf("add to map \n");
    client_list[msg.body] = user;

    //登录成功应答
    MESSAGE reply_msg;
    memset(&reply_msg, 0, sizeof(reply_msg));
    reply_msg.cmd = htonl(S2C_LOGIN_OK);

    sendto(sock,
           &reply_msg,
           sizeof(msg),
           0,
           reinterpret_cast<struct sockaddr *>(cliaddr),
           sizeof(struct sockaddr));

    int count = client_list.size();
    printf("send count= %d\n", count);
    // 发送在线人数
    sendto(sock,
           &count,
           sizeof(count),
           0,
           reinterpret_cast<struct sockaddr *>(cliaddr),
           sizeof(struct sockaddr));

    // 发送在线列表
    printf("sending user list information to %s <-> %s:%d\n",
           msg.body,
           inet_ntoa(cliaddr->sin_addr),
           ntohs(cliaddr->sin_port));

    for (auto it : client_list) {
      sendto(sock,
             &(it.second),
             sizeof(USER_INFO),
             0,
             reinterpret_cast<struct sockaddr *>(cliaddr),
             sizeof(struct sockaddr));
    }

    // 向其他用户通知新用户登录
    for (auto it : client_list) {
      if (0 == strcmp(msg.body, it.first)) {
        continue;
      }

      struct sockaddr_in peeraddr;
      memset(&peeraddr, 0, sizeof(peeraddr));
      peeraddr.sin_family      = AF_INET;
      peeraddr.sin_addr.s_addr = it.second.ip;
      peeraddr.sin_port        = it.second.port;

      msg.cmd = htons(S2C_SOMEONE_LOGIN);
      memcpy(msg.body, &user, sizeof(user));
      if (0 > sendto(sock,
                     &msg,
                     sizeof(msg),
                     0,
                     reinterpret_cast<struct sockaddr *>(&peeraddr),
                     sizeof(peeraddr))) {
        ERR_EXIT("sendto error");
      }
    }
  } else {
    // 找到用户
    printf("user %s has already login\n", msg.body);

    MESSAGE reply_msg;
    memset(&reply_msg, 0, sizeof(reply_msg));
    reply_msg.cmd = htons(S2C_ALREADY_LOGINED);

    sendto(sock,
           &reply_msg,
           sizeof(reply_msg),
           0,
           reinterpret_cast<struct sockaddr *>(cliaddr),
           sizeof(struct sockaddr));
  }
}

void do_logout(MESSAGE &msg, int sock, struct sockaddr_in *cliaddr) {
  printf("someone logout");
  USER_INFO user;
  strcpy(user.username, msg.body);
  user.ip   = cliaddr->sin_addr.s_addr;
  user.port = cliaddr->sin_port;

  // 查找用户
  USER_LIST::iterator it;
  if (0 == client_list.count(msg.body)) {
    printf("user %s is not login\n", msg.body);
    return;
  }

  printf("user %s is logout server", msg.body);

  // 从map中移除
  client_list.erase(msg.body);
  // 通知其他用户 xx用户下线

  for (auto it : client_list) {
    struct sockaddr_in peeraddr;
    peeraddr.sin_family      = PF_INET;
    peeraddr.sin_port        = htons(it.second.port);
    peeraddr.sin_addr.s_addr = htonl(it.second.ip);

    MESSAGE logout_msg;
    logout_msg.cmd = htons(S2C_SOMEONE_LOGOUT);
    memcpy(&logout_msg.body, &msg.body, sizeof(msg.body));

    sendto(sock,
           &logout_msg,
           sizeof(logout_msg),
           0,
           reinterpret_cast<struct sockaddr *>(&peeraddr),
           sizeof(peeraddr));
  }
}