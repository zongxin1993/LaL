#include "../src/lal_protocol.hh"

void solve_recv_data(SockServerManager *server, sock_client_proxy_t *client)
{
    static int count = 0;
    int revc_num = 0;
    server->recv_data(client, &revc_num, sizeof(count));
    count++;
    INF("server recv=%d\n", revc_num);
    sleep(1);
    server->send_data(client, &count, sizeof(count));
}

int main(int argc, char **argv)
{
    auto *server = new SockServerManager(7777);
    server->register_listener_callback(solve_recv_data);
    server->start();
    sleep(-1);
    delete server;
    server = nullptr;
    return 0;
}
