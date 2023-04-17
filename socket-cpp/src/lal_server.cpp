#include "lal_server.h"
#include "lal_utils.h"
#include <arpa/inet.h>
#include <cstdint>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

inline int sock_server_find_empty_slot(sock_server_t *server)
{
    int id = 0;
    for (id = 0; id < SOCK_MAX_CLIENTS; ++id) {
        if (-1 == server->clientfd_slots[id]) break;
    }
    return id;
}

sock_server_t *sock_server_init(int type, int port)
{
    int ret = -1;
    sock_server_t *server = NULL;
    int socketfd = socket(AF_INET, type, 0);
    if (socketfd < 0) {
        INF("sock sock error: create server socket failed!\n");
        return NULL;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    ret = bind(socketfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (ret < 0) {
        ERR("sock sock error : bind server socket failed! error = %d(%s) \n", errno, strerror(errno));
        close(socketfd);
        return NULL;
    }
    ret = listen(socketfd, SOMAXCONN);
    if (ret < 0) {
        ERR("sock sock error : listen server socket failed! error = %d(%s) \n", errno, strerror(errno));
        close(socketfd);
        return NULL;
    }
    server = new sock_server_t();

    if (NULL == server) {
        ERR("sock sock error: create sock_server_t instance failed!\n");
        close(socketfd);
        return NULL;
    }

    memset(server, 0, sizeof(sock_server_t));
    server->socketfd = socketfd;
    server->type = type;
    for (int i = 0; i < SOCK_MAX_CLIENTS; i++) {
        server->clientfd_slots[i] = -1;
    }
    DBG("sock_server_init(%d, %d) returns %p \n", type, port, server);
    return server;
}
void sock_server_close(sock_server_t *server)
{
    DBG("sock_server_close() ...\n");
    if (NULL == server) {
        close(server->socketfd);
        for (int i = 0; i < SOCK_MAX_CLIENTS; i++) {
            if (-1 != server->clientfd_slots[i]) {
                close(server->clientfd_slots[i]);
                server->clientfd_slots[i] = -1;
            }
        }
        delete server;
        server = NULL;
    }
    DBG("sock_server_close() successful.\n");
}
int sock_server_has_newconn(sock_server_t *server, int timeout_ms)
{
    bool result = false;
    fd_set readfds, writefds, exceptfds;
    struct timeval timeout;
    if (!server) {
        return result;
    }
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);
    FD_SET(server->socketfd, &readfds);
    timeout.tv_sec = 0;
    timeout.tv_usec = timeout_ms * 1000;

    int nsel = select(server->socketfd + 1, &readfds, NULL, NULL, &timeout);
    switch (nsel) {
    case -1:
        ERR("select() error: %s(%d) \n", strerror(errno), errno);
        result = false;
        return result;
    case 0:
        result = false;
        break;
    default:
        if (FD_ISSET(server->socketfd, &readfds)) {
            if (sock_server_find_empty_slot(server) < SOCK_MAX_CLIENTS) {
                DBG("sock server has new connection.\n");
                result = true;
            } else {
                int clientfd = 0;
                struct sockaddr_in client_addr;
                socklen_t addr_len = sizeof(struct sockaddr_in);
                clientfd = accept(server->socketfd, (struct sockaddr *)&client_addr, &addr_len);
                close(clientfd);
                INF("sock server has new connection, but client_slots is full!\n");
                result = false;
            }
        }
        break;
    }
    return result;
}
sock_client_proxy_t *sock_server_create_client(sock_server_t *server)
{
    int clientfd = 0;
    sock_client_proxy_t *client = NULL;
    int ret = 0;

    if (!server) {
        return NULL;
    }

    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    clientfd = accept(server->socketfd, (struct sockaddr *)&client_addr, &addr_len);
    if (clientfd < 0) {
        ERR("sock sock error: accept socketfd failed!\n");
        return NULL;
    }
    int id = sock_server_find_empty_slot(server);
    if (id >= SOCK_MAX_CLIENTS) {
        ERR("sock error : the client_slots is full!\n");
        close(clientfd);
        return NULL;
    }
    client = new sock_client_proxy_t();
    if (NULL == client) {
        ERR("sock error: malloc sock_client_proxy_t failed!\n");
        close(clientfd);
        return NULL;
    }
    client->slot_id = id;
    server->clientfd_slots[id] = clientfd;
    return client;
}
void sock_server_destroy_client(sock_server_t *server, sock_client_proxy_t *client)
{
    if (!server || !client) {
        return;
    }
    if (-1 != server->clientfd_slots[client->slot_id]) {
        close(server->clientfd_slots[client->slot_id]);
        server->clientfd_slots[client->slot_id] = -1;
    }

    delete client;
}
int sock_server_client_readable(sock_server_t *server, int timeout_ms)
{
    int result = false;
    int maxfd = 0;
    if (!server) {
        return result;
    }

    FD_ZERO(&(server->readfds));
    for (int i = 0; i < SOCK_MAX_CLIENTS; ++i) {
        if (server->clientfd_slots[i] != -1) {
            FD_SET(server->clientfd_slots[i], &(server->readfds));
            maxfd = (maxfd > server->clientfd_slots[i]) ? maxfd : server->clientfd_slots[i];
        }
    }

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = timeout_ms * 1000;

    int nsel = select(maxfd + 1, &(server->readfds), NULL, NULL, &timeout);
    switch (nsel) {
    case -1:
        ERR("sock error: check clients readable select failed!");
        result = false;
        break;

    case 0:
        result = false;
        break;

    default:
        result = false;
        break;
    }
    return result;
}

sock_conn_status_t sock_server_check_connection(sock_server_t *server, sock_client_proxy_t *client)
{
    sock_conn_status_t result = normal;
    if (!server || !client) {
        return result;
    }
    int clientfd = server->clientfd_slots[client->slot_id];
    if (FD_ISSET(clientfd, &server->readfds)) {
        int nread = 0;
        if (-1 != ioctl(clientfd, FIONREAD, &nread)) {
            if (nread != 0) {
                result = readable;
            } else {
                result = disconnect;
            }
        } else {
            ERR("sock server clientfd:%d ioctl error: %d, %s!\n", clientfd, errno, strerror(errno));
            result = normal;
        }
    } else {
        result = normal;
    }
    return result;
}
int sock_server_send(sock_server_t *server, const sock_client_proxy_t *sender, const void *data, int len)
{
    if (server == nullptr || sender == nullptr) return -1;

    int ret = send(server->clientfd_slots[sender->slot_id], data, len, 0);
    if (ret < 0) {
        ERR("**socket send with error: %d (%s)!!!\n", errno, strerror(errno));
    }

    DBG("sock server current send: client [%d], %d bytes, target send: %d\n", sender->slot_id, ret, len);
    return ret;
}
int sock_server_recv(sock_server_t *server, const sock_client_proxy_t *receiver, void *data, int len)
{
    if (server == nullptr || receiver == nullptr) return -1;

    int on = 1;
    int ret = recv(server->clientfd_slots[receiver->slot_id], data, len, 0);
    if (ret < 0) {
        ERR("**socket recv with error: %d (%s)!!!\n", errno, strerror(errno));
    }
    DBG("sock server current recv: %d bytes, target recv: %d\n", ret, len);
    return ret;
}