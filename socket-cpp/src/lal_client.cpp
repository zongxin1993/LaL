#include "lal_client.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <string>

sock_client_t *sock_client_init(int type, const std::string server_ip,
                                int port) {
  int socketfd = -1;
  int ret = -1;
  sock_client_t *client = NULL;
  int domain = AF_INET;
  socketfd = socket(domain, type, 0);
  if (socketfd < 0) {
    ERR("sock error: create client socket failed!\n");
    return NULL;
  }
  struct sockaddr_in serv_in;
  memset(&serv_in, 0, sizeof(serv_in));
  serv_in.sin_family = AF_INET;
  serv_in.sin_addr.s_addr = inet_addr(server_ip.c_str());
  serv_in.sin_port = port;
  ret = connect(socketfd, (struct sockaddr *)&serv_in, sizeof(serv_in));
  if (ret < 0) {
    ERR("sock error: client connect to server failed!\n");
    close(socketfd);
    return NULL;
  }
  client = new sock_client_t();
  if (NULL == client) {
    ERR("sock error: create sock_client_t instance failed!\n");
    close(socketfd);
    delete client;
    return NULL;
  }
  client->socketfd = socketfd;

  return client;
}
void sock_client_close(sock_client_t *client) {
  if (NULL != client) {
    close(client->socketfd);
    delete client;
    client = NULL;
  }
}
sock_conn_status_t sock_client_check_connect(sock_client_t *client,
                                             int timeout_ms) {
  sock_conn_status_t result = normal;
  int nsel = 0;
  fd_set rfds;
  struct timeval timeout;
  int nread = 0;

  if (!client) {
    return result;
  }

  FD_ZERO(&rfds);
  FD_SET(client->socketfd, &rfds);

  timeout.tv_sec = 0;
  timeout.tv_usec = timeout_ms * 1000;
  nsel = select(client->socketfd + 1, &rfds, NULL, NULL, &timeout);
  switch (nsel) {
    case -1:
      ERR("sock sock error: select failed!\n");
      result = normal;
      break;

    case 0:
      result = normal;
      break;

    default:
      if (FD_ISSET(client->socketfd, &rfds)) {
        if (-1 != ioctl(client->socketfd, FIONREAD, &nread)) {
          if (nread != 0) {
            result = readable;
          } else {
            result = disconnect;
          }
        } else {
          ERR("client ioctl error: %d, %s!\n", errno, strerror(errno));
          result = normal;
        }
      } else {
        result = normal;
      }
  }
  return result;
}
int sock_client_send(sock_client_t *client, const void *buf, size_t len) {
  DBG("sock_client_send\n");
  int ret = -1;
  if (!client || !buf) {
    return ret;
  }
  do {
    ret = send(client->socketfd, buf, len, 0);
  } while (0);

  return ret;
}
int sock_client_recv(sock_client_t *client, void *buf, size_t len) {
  DBG("sock_client_recv\n");
  int ret = -1;
  if (!client || !buf) {
    return ret;
  }
  do {
    ret = recv(client->socketfd, buf, len, 0);
  } while (0);

  return ret;
}