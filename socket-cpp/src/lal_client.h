#ifndef __LAL_SOCK_CLIENT_HH__
#define __LAL_SOCK_CLIENT_HH__
#include "lal_utils.h"
#include <cstddef>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _t_sock_client {
    int type;
    int socketfd;
    char *m_recv_buf;
    int m_recv_buf_len;
} sock_client_t;

sock_client_t *sock_client_init(int type, const std::string server_ip, int port);

void sock_client_close(sock_client_t *client);

sock_conn_status_t sock_client_check_connect(sock_client_t *client, int timeout_ms = 1);

int sock_client_send(sock_client_t *client, const void *buf, size_t len);

int sock_client_recv(sock_client_t *client, void *buf, size_t len);

#ifdef __cplusplus
}
#endif
#endif //__LAL_SOCK_CLIENT_HH__