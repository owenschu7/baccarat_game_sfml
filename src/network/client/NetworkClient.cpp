#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h> // for ntohl

#include "NetworkClient.h"

NetworkClient::NetworkClient() 
: m_sockfd(-1), m_connected(false) 
{
}

NetworkClient::~NetworkClient() 
{
  disconnect();
}

bool NetworkClient::connectToServer(const std::string& host, const std::string& port) 
{
  if (m_connected)
  {
    return false; // Already connected
  }

  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if (getaddrinfo(host.c_str(), port.c_str(), &hints, &res) != 0)
  {
    return false;
  }

  m_sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

  if (m_sockfd == -1)
  {
    freeaddrinfo(res);
    return false;
  }

  if (connect(m_sockfd, res->ai_addr, res->ai_addrlen) == -1)
  {
    close(m_sockfd);
    m_sockfd = -1;
    freeaddrinfo(res);
    return false;
  }

  freeaddrinfo(res);
  m_connected = true;

  // Launch the background thread to listen for incoming data
  m_listenThread = std::thread(&NetworkClient::listenLoop, this);

  return true;
}

void NetworkClient::disconnect() 
{
  if (!m_connected) return;

  m_connected = false;

  if (m_sockfd != -1)
  {
    // shutdown unblocks any pending recv() calls in the listen thread
    shutdown(m_sockfd, SHUT_RDWR); 
    close(m_sockfd);
    m_sockfd = -1;
  }

  // Safely wait for the background thread to finish
  if (m_listenThread.joinable())
  {
    m_listenThread.join();
  }
}

bool NetworkClient::isConnected() const 
{
  return m_connected;
}

bool NetworkClient::sendPacket(const PacketBuilder& packet) 
{
  if (!m_connected) return false;
  return sendAll(packet.getPtr(), packet.getSize());
}

bool NetworkClient::pollPacket(std::vector<uint8_t>& outPacket)
{
  std::lock_guard<std::mutex> lock(m_queueMutex); // modern, safer alternative to .lock() / .unlock()

  if (m_packetQueue.empty()) {
    return false;
  }

  // Copy the front packet to the output variable and remove it from the queue
  outPacket = std::move(m_packetQueue.front()); 
  m_packetQueue.pop();

  return true;
}

bool NetworkClient::sendAll(const uint8_t* data, size_t length) 
{
  size_t total = 0;
  size_t bytes_left = length;
  ssize_t n;

  while (total < length) {
    n = send(m_sockfd, data + total, bytes_left, 0);
    if (n == -1) {
      return false; // Connection dropped or error
    }
    total += static_cast<size_t>(n);
    bytes_left -= static_cast<size_t>(n);
  }
  return true;
}

void NetworkClient::listenLoop() 
{
  uint8_t temp_buffer[4096]; // Fast stack memory for raw reads
  std::vector<uint8_t> persistent_buffer; // Heap memory to hold partial packets

  while (m_connected) {
    ssize_t bytes_received = recv(m_sockfd, temp_buffer, sizeof(temp_buffer), 0);

    if (bytes_received <= 0) {
      // 0 = server closed connection, < 0 = error
      m_connected = false;
      break; 
    }

    // 1. Append new bytes to our persistent buffer
    persistent_buffer.insert(persistent_buffer.end(), temp_buffer, temp_buffer + bytes_received);

    // 2. Try to extract complete packets from the persistent buffer
    // We assume the first 4 bytes of ANY packet is a uint32_t telling us the TOTAL packet size
    while (persistent_buffer.size() >= 4) {

      // Extract the 4-byte header
      uint32_t net_size;
      std::memcpy(&net_size, persistent_buffer.data(), 4);

      // Convert from network byte order to host byte order
      uint32_t total_packet_size = ntohl(net_size);

      // Do we have all the bytes for this packet yet?
      if (persistent_buffer.size() >= total_packet_size) {

        // We have a full packet! Slice it out into a new vector.
        // (We skip the first 4 bytes because we don't need the size header anymore)
        std::vector<uint8_t> complete_packet(
          persistent_buffer.begin() + 4, 
          persistent_buffer.begin() + total_packet_size
        );

        // Hand it off to the main thread safely
        {
          std::lock_guard<std::mutex> lock(m_queueMutex);
          m_packetQueue.push(std::move(complete_packet));
        }

        // Erase the processed bytes from the persistent buffer
        persistent_buffer.erase(
          persistent_buffer.begin(), 
          persistent_buffer.begin() + total_packet_size
        );

      } else {
        // We don't have the full packet yet. Break the inner loop 
        // and go back to recv() to wait for the rest of the bytes.
        break; 
      }
    }
  }
}
