#ifndef __SOCK_PROTOCOL_HH__
#define __SOCK_PROTOCOL_HH__

#include <arpa/inet.h>

#include <cstddef>
#include <functional>
#include <mutex>
#include <thread>

#include "lal_client.h"
#include "lal_server.h"

#define SOCK_MAGIC 0x9527FFFF

typedef enum _header_type_t {
  TYPE_UNKNOWN = 0x0,
  TYPE_MESSAGE = 0x1,
} header_type_t;

typedef enum _message_type_t {
  MSG_TYPE_UNKNOWN = 0x0,
  MSG_TYPE_JSON = 0x1,
} message_type_t;

typedef struct _header_t {
  uint32_t msg_magic;
  uint32_t msg_size;
  int64_t timestamp;
  uint32_t ack_required;
  header_type_t message_type;
  uint32_t reserved[2];
} header_t;

typedef struct _message_header_t {
  uint32_t message_size;
  message_type_t message_type;

  uint32_t reserved[2];
} message_header_t;

enum package_length_t {
  LENGTH_MAIN_HEADER = sizeof(header_t),
  LENGTH_PAYLOAD_HEADER = sizeof(message_header_t),
  LENGTH_FULL_HEADER = LENGTH_MAIN_HEADER + LENGTH_PAYLOAD_HEADER
};

class GBuffer {
 public:
  GBuffer(int length) {
    m_length = length;
    m_ptr = (char *)malloc(length);
  }
  ~GBuffer() { free(m_ptr); }
  char *m_ptr;
  int m_length;

 private:
  GBuffer(const GBuffer &buffer);
  GBuffer &operator=(const GBuffer &) { return *this; }
};

class sock_packetMaker {
 public:
  sock_packetMaker(){};
  ~sock_packetMaker(){};
  std::unique_ptr<GBuffer> build_message_packet(message_type_t type,
                                                const void *payload_ptr,
                                                int payload_len);
  header_type_t parse_packet(const void *packet_ptr, int packet_len);

 private:
  sock_packetMaker(const sock_packetMaker &);
  sock_packetMaker &operator=(const sock_packetMaker &) { return *this; }
  int set_header(void *buff_p, header_type_t msg_type, int payload_len,
                 bool ack = false);
};

class SockServerManager {
 public:
  typedef std::function<void(SockServerManager *server,
                             sock_client_proxy_t *client)>
      listener_callback_t;
  typedef std::function<void(SockServerManager *server,
                             sock_client_proxy_t *client)>
      connected_callback_t;
  typedef std::function<void(SockServerManager *server,
                             sock_client_proxy_t *client)>
      disconnect_callback_t;

  SockServerManager(int port, int type = SOCK_STREAM)
      : m_type(type), m_port(port) {
    m_maker = new sock_packetMaker();
  }

  ~SockServerManager() {
    stop();
    join();
    sock_server_close(m_server);
    m_server = nullptr;
    delete m_maker;
    m_maker = nullptr;
    for (int i = 0; i < SOCK_MAX_CLIENTS; i++) {
      delete m_client_list[i];
      m_client_list[i] = nullptr;
    }
    /// bug double free,
    // delete m_pclient_;
    m_thread.reset();

    m_client = nullptr;
    m_thread = nullptr;
  }

  int start();

  void stop();

  void join();

  int send_data(const sock_client_proxy_t *client, const void *data,
                size_t len);

  int recv_data(sock_client_proxy_t *client, void *data, size_t len);

  void register_listener_callback(listener_callback_t func) {
    m_listener_callback = func;
  }

  void register_connected_callback(connected_callback_t func) {
    m_connected_callback = func;
  }

  void register_disconnected_callback(disconnect_callback_t func) {
    m_disconnected_callback = func;
  }

  sock_client_proxy_t *get_sock_client() {
    std::lock_guard<std::mutex> lock(m_client_mutex_);
    return m_client;
  }

  int send_message_packet(const sock_client_proxy_t *client,
                          message_type_t type, const void *payload_ptr,
                          int payload_len);

 private:
  sock_server_t *m_server = nullptr;
  sock_client_proxy_t *m_client_list[SOCK_MAX_CLIENTS] = {nullptr};
  sock_client_proxy_t *m_client = nullptr;
  std::mutex m_client_mutex_;
  int m_ncount = 0;
  bool m_done = false;
  int m_port = 9527;
  int m_type = 0;
  sock_packetMaker *m_maker = nullptr;
  std::unique_ptr<std::thread> m_thread = nullptr;
  listener_callback_t m_listener_callback = nullptr;
  connected_callback_t m_connected_callback = nullptr;
  disconnect_callback_t m_disconnected_callback = nullptr;

 private:
  SockServerManager(const SockServerManager &);
  SockServerManager &operator=(const SockServerManager &) { return *this; };
  static void task(SockServerManager *ptr) { ptr->mainloop(); }
  void mainloop();
  int check_new_connection();
  int check_new_message();
};

class SockClinetManager {
 public:
  typedef std::function<void(SockClinetManager *client)> listener_callback_t;
  typedef std::function<void(SockClinetManager *client)> connected_callback_t;
  typedef std::function<void(SockClinetManager *client)> disconnect_callback_t;

  SockClinetManager(std::string sockaddr, int port, int type = SOCK_STREAM,
                    float timeout = 2)
      : m_sockaddr(sockaddr), m_port(port), m_timeout(timeout), m_type(type) {
    m_maker = new sock_packetMaker();
  };

  ~SockClinetManager() {
    stop();
    join();
    sock_client_close(m_client);
    m_client = nullptr;
    delete m_maker;
    m_maker = nullptr;
    m_thread.reset();
    m_thread = nullptr;
  }

  int start();

  void stop();

  void join();

  int send_data(const void *data, size_t len);

  int recv_data(void *data, size_t len);

  void register_listener_callback(listener_callback_t func) {
    m_listener_callback = func;
  }

  void register_connected_callback(connected_callback_t func) {
    m_connected_callback = func;
  }

  void register_disconnected_callback(disconnect_callback_t func) {
    m_disconnected_callback = func;
  }

  sock_client_t *get_sock_client() { return m_client; }

  int send_message_packet(message_type_t type, const void *payload_ptr,
                          int payload_len);

 private:
  sock_client_t *m_client = nullptr;
  int m_port = 9527;
  std::string m_sockaddr;
  int m_type = -1;
  bool m_done = false;
  float m_timeout = 2;
  sock_packetMaker *m_maker = nullptr;
  std::unique_ptr<std::thread> m_thread = nullptr;
  listener_callback_t m_listener_callback = nullptr;
  connected_callback_t m_connected_callback = nullptr;
  disconnect_callback_t m_disconnected_callback = nullptr;

 private:
  SockClinetManager(const SockClinetManager &);
  SockClinetManager &operator=(const SockClinetManager &) { return *this; };
  static void task(SockClinetManager *ptr) { ptr->mainloop(); }
  void mainloop();
  int check_new_connection();
  int check_new_message();
  int connect_to_server();
};

#endif  //__SOCK_PROTOCOL_HH__