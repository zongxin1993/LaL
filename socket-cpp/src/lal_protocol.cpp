#include "lal_protocol.hh"

#include <unistd.h>

#include <cstring>

#include "lal_client.h"
#include "lal_server.h"
#include "lal_utils.h"

std::unique_ptr<GBuffer> sock_packetMaker::build_message_packet(
    message_type_t type, const void *payload_ptr, int payload_length) {
  std::unique_ptr<GBuffer> buff_ptr(
      new GBuffer(LENGTH_FULL_HEADER + payload_length));
  set_header(buff_ptr->m_ptr, TYPE_MESSAGE,
             LENGTH_PAYLOAD_HEADER + payload_length);
  message_header_t *header_ptr =
      (message_header_t *)(buff_ptr->m_ptr + LENGTH_MAIN_HEADER);
  header_ptr->message_type = type;
  header_ptr->message_size = LENGTH_PAYLOAD_HEADER + payload_length;
  memcpy(buff_ptr->m_ptr + LENGTH_FULL_HEADER, payload_ptr, payload_length);
  return buff_ptr;
}

int sock_packetMaker::set_header(void *buff_ptr, header_type_t msg_type,
                                 int payload_length, bool ack) {
  header_t *header_ptr = (header_t *)buff_ptr;
  struct timeval current;
  gettimeofday(&current, NULL);
  uint64_t timestamp =
      (uint64_t)(current.tv_sec) * 1000000 + (uint64_t)current.tv_usec;

  header_ptr->msg_magic = SOCK_MAGIC;
  header_ptr->msg_size = (uint32_t)(LENGTH_MAIN_HEADER + payload_length);
  header_ptr->timestamp = timestamp;
  header_ptr->ack_required = ack;

  return LENGTH_MAIN_HEADER;
}

header_type_t sock_packetMaker::parse_packet(const void *packet_ptr,
                                             int packet_len) {
  int len = packet_len;
  char *ptr = (char *)packet_ptr;

  while (len > LENGTH_MAIN_HEADER) {
    header_t *header_ptr = (header_t *)(ptr);

    if (header_ptr->msg_size < LENGTH_MAIN_HEADER ||
        (uint32_t)len < header_ptr->msg_size) {
      DBG("receive wrong size: %d, len = %d", header_ptr->msg_size, len);
      break;
    }

    message_header_t *msg_header_ptr = (message_header_t *)(header_ptr);

    char *payload_ptr = ptr + LENGTH_MAIN_HEADER;

    switch (header_ptr->message_type) {
      case TYPE_MESSAGE: {
        if (len < LENGTH_FULL_HEADER) {
          DBG("not enough data to parse message header");
          break;
        }
        message_header_t *message_header = (message_header_t *)(payload_ptr);
        int message_size = message_header->message_size;

        if (header_ptr->msg_size !=
            (uint32_t)(message_size + LENGTH_MAIN_HEADER)) {
          DBG("message length check failed, header_size is %d, message_size is "
              "%d",
              header_ptr->msg_size, message_size);
          break;
        }
        return TYPE_MESSAGE;
      }
      default:
        DBG("receive unknown type: %d", header_ptr->message_type);
        return TYPE_UNKNOWN;
    }
    ptr += header_ptr->msg_size;
    len -= header_ptr->msg_size;
  }
  return TYPE_UNKNOWN;
}

/*
---------------------------------------SockServer---------------------------------------
*/

int SockServerManager::start() {
  m_done = false;
  m_server = sock_server_init(m_type, m_port);
  if (m_server == nullptr) return -1;
  for (int i = 0; i < SOCK_MAX_CLIENTS; i++) {
    delete m_client_list[i];
    m_client_list[i] = nullptr;
  }
  m_thread.reset(new std::thread(task, this));
  return 0;
}
void SockServerManager::stop() { m_done = true; }
void SockServerManager::join() {
  if (m_thread->joinable() && m_thread) {
    m_thread->join();
  }
}
void SockServerManager::mainloop() {
  while (!m_done && (m_server != nullptr)) {
    check_new_connection();
    check_new_message();
  }
}

int SockServerManager::send_data(const sock_client_proxy_t *client,
                                 const void *data, size_t len) {
  if (client == nullptr || m_server == nullptr) return -1;

  return sock_server_send(m_server, client, data, len);
}

int SockServerManager::send_message_packet(const sock_client_proxy_t *client,
                                           message_type_t type,
                                           const void *payload_ptr,
                                           int payload_len) {
  if (client == nullptr || m_server == nullptr) return -1;
  std::unique_ptr<GBuffer> buff =
      m_maker->build_message_packet(type, payload_ptr, payload_len);
  return send_data(client, buff->m_ptr, (size_t)buff->m_length);
}

int SockServerManager::recv_data(sock_client_proxy_t *client, void *data,
                                 size_t len) {
  if (m_server == nullptr || client == nullptr) return -1;
  int length = sock_server_recv(m_server, client, data, len);
  return length;
}

int SockServerManager::check_new_connection() {
  if (sock_server_has_newconn(m_server, 0)) {
    auto client = sock_server_create_client(m_server);
    if (client == nullptr) return -1;
    m_client_list[client->slot_id] = client;
    // m_server->client_slots[client->id] = client->id;
    m_ncount++;
    if (m_connected_callback) m_connected_callback(this, client);
  }
  return m_ncount;
}
int SockServerManager::check_new_message() {
  if (sock_server_client_readable(m_server, 3) != true) return -1;
  for (int i = 0; i < SOCK_MAX_CLIENTS; i++) {
    if (m_client_list[i] == nullptr) continue;
    switch (sock_server_check_connection(m_server, m_client_list[i])) {
      case readable:
        if (m_listener_callback) {
          m_listener_callback(this, m_client_list[i]);
        }
        break;
      case disconnect:
        if (m_disconnected_callback) {
          m_disconnected_callback(this, m_client_list[i]);
        }
        {
          std::lock_guard<std::mutex> lock(m_client_mutex_);
          sock_server_destroy_client(m_server, m_client_list[i]);
          if (m_client == m_client_list[i]) m_client = nullptr;
        }

        m_client_list[i] = nullptr;
        m_ncount--;
        if (m_ncount == 0) return i + 1;
        break;
      case normal:
        break;
    }
  }
  // chose a new default client
  {
    std::lock_guard<std::mutex> lock(m_client_mutex_);
    if (m_client == nullptr) {
      for (int i = 0; i < SOCK_MAX_CLIENTS; i++) {
        if (m_client_list[i] == nullptr) continue;
        m_client = m_client_list[i];
        DBG("set client %d as default client\n", m_client_list[i]->slot_id);
        break;
      }
    }
  }
  return 0;
}
/*
---------------------------------SockServer end---------------------------------
*/

/*
---------------------------------SockClinetManager------------------------------
*/
int SockClinetManager::start() {
  m_done = false;
  m_thread.reset(new std::thread(task, this));
  return 0;
}
void SockClinetManager::stop() { m_done = true; }

void SockClinetManager::join() {
  if (m_thread && m_thread->joinable()) {
    m_thread->join();
  }
}
void SockClinetManager::mainloop() {
  connect_to_server();

  while (!m_done) {
    check_new_message();
  }
}
int SockClinetManager::connect_to_server() {
  int timeout_cnt = 0;
  const int MAX_TIMEOUT_LOG = 30;

  while (!m_client) {
    m_client = sock_client_init(m_type, m_sockaddr, m_port);
    if (!m_client) {
      timeout_cnt++;

      usleep(m_timeout * 1000 * 1000);
      // TODO something
      if (timeout_cnt > MAX_TIMEOUT_LOG && !m_done) {
        continue;
      } else if (timeout_cnt == MAX_TIMEOUT_LOG) {
      } else {
      }
    }

    if (m_done) {
      return 0;
    }
  }
  if (m_connected_callback) {
    m_connected_callback(this);
  }
  return 0;
}
int SockClinetManager::send_data(const void *data, size_t len) {
  if (!m_client) {
    return -1;
  }
  int length = sock_client_send(m_client, data, len);
  return length;
}
int SockClinetManager::send_message_packet(message_type_t type,
                                           const void *payload_ptr,
                                           int payload_len) {
  if (payload_ptr == nullptr) return -1;
  std::unique_ptr<GBuffer> buff =
      m_maker->build_message_packet(type, payload_ptr, payload_len);
  return send_data(buff->m_ptr, (size_t)buff->m_length);
}
int SockClinetManager::recv_data(void *data, size_t len) {
  if (!m_client) {
    return -1;
  }
  int ret = sock_client_recv(m_client, data, len);

  return ret;
}
int32_t SockClinetManager::check_new_message() {
  switch (sock_client_check_connect(m_client)) {
    case readable:
      if (m_listener_callback) {
        m_listener_callback(this);
      }
      break;

    case disconnect:
      if (m_disconnected_callback) {
        m_disconnected_callback(this);
      }

      // TODO: fix bug, client_ is not NULL
      sock_client_close(m_client);
      m_client = NULL;
      connect_to_server();
      break;

    default:
      break;
  }
  return 0;
}
