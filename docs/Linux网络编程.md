# `Linux`网络编程
## 网络模型
`OSI`七层网络模型
![OSI网络模型](./images/OSI七层网络模型.png)
`TCPIP`网络模型:
![TCPIP](./images/TCPIP网络模型.png)
![以太网帧](./images/以太网帧.png)
`ICMP`协议用于传递差错信息、时间、回显、网络信息等控制数据。
![ICMP](./images/ICMP协议.png)
![ARP](./images/ARP地址解析.png)
![IP数据报文格式](./images/IP数据报文格式.png)
![路由](./images/路由.png)
![tcp报文格式](./images/TCP报文格式.png)
![tcp三次握手](./images/TCP三次握手.png)
![tcp四次挥手](./images/TCP四次挥手.png)
`TCP`可靠性保证：
1. 应用数据被分割成`TCP`认为最适合的段传递给`IP`层；
2. `TCP`发送一个报文之后，启动一个定时器，等待目的端确认收到这个报文段，如果不能及时收到这个确认，将重发这个报文段；
3. 当`TCP`收到发自另一个`TCP`端的数据时，将发送一个确认。这个确认延迟几分之一秒发送；
4. `TCP`将保持它的首部和数据的检验和，如果检验和不对丢弃数据不发送确认；
5. `TCP`对收到的报文重新排序；
6. `IP`报文段可能重复，`TCP`具有报文段去重的能力；
7. `TCP`提供报文控制，每个`TCP`连接端都提供一个缓冲区。

滑动窗口协议：
1. 通过接收窗口(`rwnd`): 预防应用程序发送的数据超过对方的缓冲区，接收方使用流量控制；
2. 拥塞窗口(`cwnd`)：预防发送的数据超过网络的承载能力，发送方流量控制；
3. 发送窗口取两者较小值；
4. 慢启动阈值(`ssthresh: slow start threshold`)
5. 慢启动阶段： `cwnd`从`1`增长到`ssthresh`；
6. 拥塞避免阶段： `cwnd`线性增长直至拥塞，将`cwnd=1, ssthresh减半`。

`UDP`特点：
无连接，不可靠，更加高效。
![UDP数据格式](./images/UDP数据格式.png)
### `scoket`
`socket`可看做用户进程与内核网络协议栈的编程接口。
不仅可以在本机通信，也可以在不同主机的进程间通信。
通用地址结构：
```cpp
strcut sockaddr_in{
  uint8_t sin_len;
  sa_family_t sin_famliy;
  char sa_data[14];
};
```

网络字节序为大端字节序。
#### 套接字类型
1. 流式套接字(`SOCK_STREAM`): 面向连接的可靠的数据连接服务，数据无差错，无重复发送，且按顺序接收；
2. 数据报式套接字(`SOCK_DGRAM`)：无连接服务，可靠性无保证。
3. 原始套接字(`SOCK_RAW`)

`echo server`:
```cpp
/**
 * @file echosrv.cc
 * @author koritafei (koritafei@gmail.com)
 * @brief
 * @version 0.1
 * @date 2021-09-02
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <bits/types/error_t.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdlib>

#define ERR_EXIT(m)                                                            \
  do {                                                                         \
    perror(m);                                                                 \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

void do_service(int conn) {
  char buf[1024];
  for (;;) {
    memset(buf, 0, 1024);
    int ret = read(conn, buf, 1024);
    if (0 == ret) {
      printf("client close\n");
      break;
    }
    fputs(buf, stdout);
    write(conn, buf, ret);
  }
}

int main(int argc, char **argv) {
  int listenfd;
  if (0 > (listenfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))) {
    ERR_EXIT("socket error");
  }

  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family      = AF_INET;
  servaddr.sin_port        = htons(8899);
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

  int on = 1;

  if (0 > setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) {
    ERR_EXIT("setsockopt error");
  }

  if (0 > bind(listenfd,
               reinterpret_cast<struct sockaddr *>(&servaddr),
               sizeof(servaddr))) {
    ERR_EXIT("bind error");
  }

  if (0 > listen(listenfd, SOMAXCONN)) {
    ERR_EXIT("listen error");
  }

  struct sockaddr_in peeraddr;
  socklen_t          addrlen = sizeof(peeraddr);
  int                conn;

  pid_t pid;

  for (;;) {
    if (0 > (conn = accept(listenfd,
                           reinterpret_cast<struct sockaddr *>(&peeraddr),
                           &addrlen))) {
      ERR_EXIT("accept error");
    }

    printf("ip = %s, port = %d connect\n",
           inet_ntoa(peeraddr.sin_addr),
           ntohs(peeraddr.sin_port));
    pid = fork();
    if (-1 == pid) {
      ERR_EXIT("fork error");
    }

    if (0 == pid) {
      close(listenfd);
      do_service(conn);
      exit(EXIT_SUCCESS);
    } else {
      close(conn);
    }
  }

  close(conn);
  close(listenfd);

  return 0;
}
```
`echo client`:
```cpp
/**
 * @file echocli.cc
 * @author koritafei (koritafei@gmail.com)
 * @brief
 * @version 0.1
 * @date 2021-09-02
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <bits/types/error_t.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define ERR_EXIT(m)                                                            \
  do {                                                                         \
    perror(m);                                                                 \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

int main(int argc, char **argv) {
  int listenfd;

  if (0 > (listenfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))) {
    ERR_EXIT("socket error");
  }

  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));

  servaddr.sin_family      = AF_INET;
  servaddr.sin_port        = ntohs(8899);
  servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

  if (-1 == connect(listenfd,
                    reinterpret_cast<struct sockaddr *>(&servaddr),
                    sizeof(servaddr))) {
    ERR_EXIT("connect error");
  }

  char buf[1024];
  for (;;) {
    memset(buf, 0, 1024);
    if (NULL != fgets(buf, 1024, stdin)) {
      write(listenfd, buf, strlen(buf));
    }

    if (0 != read(listenfd, buf, 1024)) {
      printf("%s", buf);
    } else {
      printf("server close\n");
      break;
    }
  }

  close(listenfd);

  return 0;
}
```
多进程聊天：
`server`:
```cpp
/**
 * @file peerserv.cc
 * @author koritafei (koritafei@gmail.com)
 * @brief
 * @version 0.1
 * @date 2021-09-02
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdlib>

#define MAXLINE 1024
#define ERR_EXIT(m)                                                            \
  do {                                                                         \
    perror(m);                                                                 \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

void handler(int sig) {
  printf("recv a sig = %d\n", sig);
  exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
  int sockfd;
  if (0 > (sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))) {
    ERR_EXIT("socket error");
  }

  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family      = AF_INET;
  servaddr.sin_port        = htons(8899);
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

  // bind
  if (0 > bind(sockfd,
               reinterpret_cast<struct sockaddr *>(&servaddr),
               sizeof(sockaddr))) {
    ERR_EXIT("bind error");
  }

  int on = 1;
  if (0 > setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) {
    ERR_EXIT("setsockopt error");
  }

  // listen
  if (0 > listen(sockfd, SOMAXCONN)) {
    ERR_EXIT("listen error");
  }

  // accept
  int                conn;
  struct sockaddr_in peeraddr;
  socklen_t          peerlen = sizeof(peeraddr);

  if (0 > (conn = accept(sockfd,
                         reinterpret_cast<struct sockaddr *>(&peeraddr),
                         &peerlen))) {
    ERR_EXIT("accept error");
  }

  printf("connenct ip = %s, port = %d\n",
         inet_ntoa(peeraddr.sin_addr),
         ntohs(peeraddr.sin_port));
  pid_t pid = fork();
  if (-1 == pid) {
    ERR_EXIT("fork error");
  }

  if (0 == pid) {
    signal(SIGUSR1, handler);
    char sendbuf[1024] = {0};
    while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL) {
      int ret = write(conn, sendbuf, strlen(sendbuf));
      if (ret == 0) {
        printf("peer close\n");
        close(sockfd);
        break;
      }
      if (0 > ret) {
        printf("peer close\n");
        close(sockfd);
        break;
      }
      memset(sendbuf, 0, MAXLINE);
    }
    exit(EXIT_SUCCESS);
  } else {
    // 父进程获取数据
    char buf[MAXLINE];
    for (;;) {
      memset(buf, 0, MAXLINE);
      int ret = read(conn, buf, MAXLINE);
      if (ret == 0) {
        printf("client close\n");
        close(conn);
        close(sockfd);
        kill(pid, SIGUSR1);
        break;
      }
      if (-1 == ret) {
        close(conn);
        close(sockfd);
        kill(pid, SIGUSR1);
        ERR_EXIT("read error");
      }
      fputs(buf, stdout);
    }

    kill(pid, SIGUSR1);
    exit(EXIT_SUCCESS);
  }

  close(conn);
  close(sockfd);

  return 0;
}
```
`client`:
```cpp
/**
 * @file peerclient.cc
 * @author koritafei (koritafei@gmail.com)
 * @brief
 * @version 0.1
 * @date 2021-09-02
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
#include <sys/types.h>
#include <unistd.h>

#include <csignal>

#define MAXLINE 1024

#define ERR_EXIT(m)                                                            \
  do {                                                                         \
    perror(m);                                                                 \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

void handler(int sig) {
  printf("recv a sig = %d\n", sig);
  exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
  int sockfd;

  if (0 > (sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))) {
    ERR_EXIT("socket error");
  }

  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family      = AF_INET;
  servaddr.sin_port        = htons(8899);
  servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

  if (-1 == connect(sockfd,
                    reinterpret_cast<struct sockaddr *>(&servaddr),
                    sizeof(servaddr))) {
    ERR_EXIT("connect error");
  }

  pid_t pid = fork();
  if (-1 == pid) {
    ERR_EXIT("fork error");
  }

  if (0 == pid) {
    // 子进程接收数据
    char buf[MAXLINE];
    for (;;) {
      int ret = read(sockfd, buf, MAXLINE);
      if (0 < ret) {
        printf("%s", buf);
      }
      if (0 == ret) {
        kill(getppid(), SIGUSR1);
        close(sockfd);
        printf("server close\n");
        break;
      }
      if (0 > ret) {
        kill(getppid(), SIGUSR1);
        close(sockfd);
        ERR_EXIT("read error");
      }
      memset(buf, 0, MAXLINE);
    }
  } else {
    signal(SIGUSR1, handler);
    char buf[MAXLINE];
    for (;;) {
      memset(buf, 0, MAXLINE);
      if (NULL != fgets(buf, MAXLINE, stdin)) {
        int ret = write(sockfd, buf, strlen(buf));
        if (ret == 0) {
          printf("peer close\n");
          close(sockfd);
          break;
        }
        if (0 > ret) {
          printf("peer close\n");
          close(sockfd);
          break;
        }
      }
    }
  }

  return 0;
}
```
### 流协议与粘包
![粘包](./images/粘包.png)
粘包的原因：
1. 出现粘包现象的原因是多方面的，它既可能由发送方造成，也可能由接收方造成。
2. 发送方引起的粘包是由TCP协议本身造成的，TCP为提高传输效率，发送方往往要收集到足够多的数据后才发送一包数据。若连续几次发送的数据都很少，通常TCP会根据优化算法把这些数据合成一包后一次发送出去，这样接收方就收到了粘包数据。
3. 接收方引起的粘包是由于接收方用户进程不及时接收数据，从而导致粘包现象。这是因为接收方先把收到的数据放在系统接收缓冲区，用户进程从该缓冲区取数据，若下一包数据到达时前一包数据尚未被用户进程取走，则下一包数据放到系统接收缓冲区时就接到前一包数据之后，而用户进程根据预先设定的缓冲区大小从系统接收缓冲区取数据，这样就一次取到了多包数据。

粘包解决方案：
1. 定长包；
2. 包尾增加`\r\n(ftp)`;
3. 包头增加包体长度；
4. 更复杂的应用层协议。




