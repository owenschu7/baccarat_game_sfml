#pragma once

#include <vector>
#include <unordered_map>
#include <poll.h>
#include <cstdint>

#include "../../core/GameEvents.h"
#include "SessionManager.h"

class ServerApplication
{
public:
  ServerApplication(const char* port);
  ~ServerApplication();

  void run();
  void stop();

  int sendAll(int sockfd, const char *buf, int *len);

private:
  bool setupServer();
  void acceptNewClient();
  void handleClientData(size_t pollIndex);
  void disconnectClient(size_t pollIndex);
  void sendEventToClient(int client_fd, const GameEvent& event);

  const char* m_port;
  int m_serverSocket;
  bool m_isRunning;

  std::vector<struct pollfd> m_pollFds;
  std::unordered_map<int, std::vector<uint8_t>> m_clientBuffers;

  TableManager m_tableManager;
  SessionManager m_sessionManager;
};
