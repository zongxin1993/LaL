#include "../src/lal_protocol.hh"
#include <arpa/inet.h>
#include <string>

void solve_recv_data(SockClinetManager *client)
{
    static int count = 0;
    int revc_num = 0;
    client->recv_data(&revc_num, sizeof(count));
    count++;
    INF("client recv=%d\n", revc_num);
    sleep(1);
    client->send_data(&count, sizeof(count));
}

void new_connection(SockClinetManager *client)
{
    int hello = 9999;
    INF("new client\n");
    client->send_data(&hello, sizeof(hello));
}

int main(int argc, char **argv)
{
    auto *client = new SockClinetManager(std::string(argv[1]), 7777);
    client->register_listener_callback(solve_recv_data);
    client->register_connected_callback(new_connection);

    client->start();
    int num = 0;
    client->send_data(&num, sizeof(num));
    sleep(-1);
    delete client;
    client = nullptr;
    return 0;
}
