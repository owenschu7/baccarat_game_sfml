#pragma once

#include <string>
#include <thread>
#include <queue>
#include <mutex>
#include <vector>
#include <atomic>
#include <cstdint>

#include "../PacketBuilder.h"

class NetworkClient
{
public:
  NetworkClient();
  ~NetworkClient();

  // Prevent copying of the network client
  NetworkClient(const NetworkClient&) = delete;
  NetworkClient& operator=(const NetworkClient&) = delete;

  //connection management
  bool connectToServer(const std::string &host, const std::string &port);
  void disconnect();
  bool isConnected() const;

  //sending binary data via PacketBuilder
  bool sendPacket(const PacketBuilder &packet);

  //thread-safe way for the main loop (application) to grab received packets
  //returns true if a packet was popper from the queue, false if empty
  bool pollPacket(std::vector<uint8_t> &outPacket);

private:
  //core networking variables
  int m_sockfd;
  std::atomic<bool> m_connected; //atomic so both threads can read/write it safely

  // threading variables
  std::thread m_listenThread;
  std::mutex m_queueMutex;
  std::queue<std::vector<uint8_t>> m_packetQueue;

  //background thread function
  void listenLoop();

  //helper for guaranteed binary delivery
  bool sendAll(const uint8_t *data, size_t length);
};

