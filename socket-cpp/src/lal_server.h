#ifndef __LAL_SOCK_SERVER_HH__
#define __LAL_SOCK_SERVER_HH__

#include <sys/select.h>

#include <cstddef>

#include "lal_utils.h"

#define SOCK_MAX_CLIENTS 255

typedef struct _t_sock_server {
  int type;
  int socketfd;
  int clientfd_slots[SOCK_MAX_CLIENTS];
  fd_set readfds;
} sock_server_t;

typedef struct _t_sock_proxy_client_t {
  int slot_id;
  char *m_recv_buff;
  int m_recv_buff_len;
} sock_client_proxy_t;

#ifdef __cplusplus
extern "C" {
#endif

sock_server_t *sock_server_init(int type, int port);

void sock_server_close(sock_server_t *server);

int sock_server_has_newconn(sock_server_t *server, int timeout_ms = 1);

sock_client_proxy_t *sock_server_create_client(sock_server_t *server);

void sock_server_destroy_client(sock_server_t *server,
                                sock_client_proxy_t *client);

int sock_server_client_readable(sock_server_t *server, int timeout_ms = 1);

sock_conn_status_t sock_server_check_connection(sock_server_t *server,
                                                sock_client_proxy_t *client);

int sock_server_send(sock_server_t *server, const sock_client_proxy_t *sender,
                     const void *data, int len);

int sock_server_recv(sock_server_t *server, const sock_client_proxy_t *receiver,
                     void *data, int len);

#ifdef __cplusplus
}
#endif  // __cplusplus
#endif  //__LAL_SOCK_SERVER_HH__