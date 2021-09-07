/**
 * @file chatpub.h
 * @author koritafei (koritafei@gmail.com)
 * @brief
 * @version 0.1
 * @date 2021-09-05
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef __CHATPUB_H__
#define __CHATPUB_H__

#include <algorithm>
#include <unordered_map>

// C2S
#define C2S_LOGIN       0x01
#define C2S_LOGOUT      0x02
#define C2S_ONLINE_USER 0x03

#define MSG_LEN 512

// S2C
#define S2C_LOGIN_OK        0x01
#define S2C_ALREADY_LOGINED 0x02
#define S2C_SOMEONE_LOGIN   0x03
#define S2C_SOMEONE_LOGOUT  0x04
#define S2C_ONLINE_USER     0x05

// C2C
#define C2C_CHAT 0x06

// msg
typedef struct message {
  int  cmd;
  char body[MSG_LEN];
} MESSAGE;

typedef struct user_info {
  char           username[16];
  unsigned int   ip;
  unsigned short port;
} USER_INFO;

typedef struct chat_msg {
  char user_name[16];
  char msg[100];
} CHAT_MSG;

typedef std::unordered_map<char *, USER_INFO> USER_LIST;

#endif /* __CHATPUB_H__ */
