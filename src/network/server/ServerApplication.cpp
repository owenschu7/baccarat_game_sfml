#include "ServerApplication.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "../PacketBuilder.h"
#include "../PacketParser.h"
#include "SessionManager.h"
#include "TableManager.h"

#include "../../core/Debug.h"

// Inside ServerApplication constructor
ServerApplication::ServerApplication(const char* port) 
    : m_port(port), 
      m_tableManager(), // 1. Initialize the TableManager first
      m_sessionManager(
          [this](int fd, const GameEvent& event){ this->sendEventToClient(fd, event); }, 
          m_tableManager    // 2. Pass it as the second argument here!
      )
{
}

ServerApplication::~ServerApplication() 
{
  stop();
  // delete m_tableManager;
}

void ServerApplication::run()
{
  DEBUG_PRINT << "ServerApplication.cpp: run():\n";
  if (!setupServer())
  {
    std::cerr << "Failed to start server on port" << m_port << "\n";
    return;
  }

  m_isRunning = true;
  DEBUG_PRINT << "  Server is running....\n";

  while (m_isRunning)
  {
    //eventually switch to epoll
    //also 100 is a delay to set the speed of the server, -1 to set it to trigger when data arrives, 100 sets the delay to trigger every 100 ms
    int activity = poll(m_pollFds.data(), m_pollFds.size(), 100);
    if (activity < 0) 
    {
      perror("poll error");
      break;
    }

    for (int i = m_pollFds.size() - 1; i >= 0; i--)
    {
      if ((m_pollFds[i].revents & POLLIN) == 0)
      {
        continue;
      }
      if (m_pollFds[i].fd == m_serverSocket)
      {
        acceptNewClient();
      }
      else
      {
        handleClientData(i);
      }
    }
  }
}

void ServerApplication::stop()
{
  DEBUG_PRINT << "ServerApplication.cpp: stop():\n";
  m_isRunning = false;
  for (auto& pfd : m_pollFds)
  {
    close(pfd.fd);
  }
  m_pollFds.clear();
  m_clientBuffers.clear();
}


bool ServerApplication::setupServer()
{
  DEBUG_PRINT << "ServerApplication.cpp: setupServer():\n";
  struct addrinfo hints, *res;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if (getaddrinfo(NULL, m_port, &hints, &res) != 0)
  {
    perror("getaddrinfo");
    return false;
  }

  m_serverSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (m_serverSocket == -1)
  {
    perror("socket");
    freeaddrinfo(res);
    return false;
  }

  int yes = 1;
  if (setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
  {
    perror("setsockopt");
    close(m_serverSocket);
    freeaddrinfo(res);
    return false;
  }

  if (bind(m_serverSocket, res->ai_addr, res->ai_addrlen) == -1)
  {
    perror("bind");
    close(m_serverSocket);
    freeaddrinfo(res);
    return false;
  }

  freeaddrinfo(res);

  // start listening. allow up to 10 pending connections in the OS backlog.
  if (listen(m_serverSocket, 10) == -1)
  {
    perror("listen");
    close(m_serverSocket);
    return false;
  }

  // add server socket to poll

  // register the main server socket with our polling system so we know when people connect
  pollfd server_pollfd;
  server_pollfd.fd = m_serverSocket;
  server_pollfd.events = POLLIN; // we care about incoming data/connections
  server_pollfd.revents = 0;
  m_pollFds.push_back(server_pollfd);

  return true;
}

// accepts a new incoming TCP connection and sets up their buffer.
void ServerApplication::acceptNewClient()
{
  DEBUG_PRINT << "ServerApplication.cpp: acceptNewClient():\n";
  struct sockaddr_storage their_addr;
  socklen_t addr_size = sizeof(their_addr);

  int new_fd = accept(m_serverSocket, (struct sockaddr *)&their_addr, &addr_size);
  if (new_fd == -1)
  {
    perror("accept");
  }
  else
  {
    DEBUG_PRINT << "  got a connecton on FD: " << new_fd << std::endl;

    pollfd client_pollfd;
    client_pollfd.fd = new_fd;
    client_pollfd.events = POLLIN;
    client_pollfd.revents = 0;
    m_pollFds.push_back(client_pollfd);

    //initialize an empty vector buffer for this client
    m_clientBuffers[new_fd] = std::vector<uint8_t>();

    m_sessionManager.onClientConnected(new_fd);
  }
}


// reads raw bytes from a client and frames them into discrete packets.
// pollIndex - The index of the client in the m_pollFds vector.
void ServerApplication::handleClientData(size_t pollIndex) 
{
  DEBUG_PRINT << "ServerApplication.cpp: handleClientData():\n";

  int client_fd = m_pollFds[pollIndex].fd;
  uint8_t buf[1024]; // temp buffer to catch incoming OS bytes

  ssize_t bytes_received = recv(client_fd, buf, sizeof(buf), 0);

  if (bytes_received <= 0) 
  {
    disconnectClient(pollIndex);
  } 
  else 
  {
    // 1. Grab this client's personal binary waiting room
    auto& buffer = m_clientBuffers[client_fd];

    // 2. Safely append the newly received bytes to the end of their vector
    buffer.insert(buffer.end(), buf, buf + bytes_received);

    // 3. THE FRAMING LOOP
    // We need AT LEAST 2 bytes to even know how big the packet is supposed to be.
    while (buffer.size() >= 2) 
    {
      // A. Read the first 2 bytes (The Header)
      uint16_t network_size;
      memcpy(&network_size, buffer.data(), sizeof(uint16_t));

      // B. Convert it from Network format back to a normal integer
      uint16_t payload_size = ntohs(network_size);

      // C. Do we have the entire packet yet? (2 header bytes + the payload size)
      if (buffer.size() < 2 + payload_size) 
      {
        // We only received half the packet. Break the loop and wait 
        // for poll() to wake us up again when the rest of the bytes arrive!
        break; 
      }

      // --- WE GOT A 100% COMPLETE BINARY PACKET! ---

      // D. Extract just the payload (skip the first 2 bytes)
      std::vector<uint8_t> completePacket(
        buffer.begin() + 2, 
        buffer.begin() + 2 + payload_size
      );

      // E. Erase the processed packet from the front of the waiting room
      buffer.erase(buffer.begin(), buffer.begin() + 2 + payload_size);


      // F. Pass the pure binary payload up to Layer 3!
      //
      // [LAYER 3 HOOK]: m_tableManager->routePacket(client_fd, completePacket);
      // 1. OPEN THE BOX: Ask the PacketParser to turn the raw bytes into a GameEvent
      GameEvent receivedEvent = PacketParser::parse(completePacket);

      // 2. PASS TO LOGIC: Hand the neat struct to the SessionManager!
      m_sessionManager.handleClientMessage(client_fd, receivedEvent);
    }
  }
}

void ServerApplication::disconnectClient(size_t pollIndex) 
{
  int client_fd = m_pollFds[pollIndex].fd;
  DEBUG_PRINT << "ServerApplication.cpp: disconnectClient():\n";
  DEBUG_PRINT << "  Client FD: " << client_fd << " disconnected." << "\n";

  m_sessionManager.onClientDisconnected(client_fd);

  close(client_fd);
  m_clientBuffers.erase(client_fd);
  m_pollFds.erase(m_pollFds.begin() + pollIndex);
}

int ServerApplication::sendAll(int sockfd, const uint8_t *buf, size_t &len) 
{
  DEBUG_PRINT << "ServerApplication.cpp: sendAll():\n";
  size_t total = 0;
  size_t bytes_left = len;
  ssize_t n = 0;

  while (total < len) 
  {
    n = send(sockfd, buf + total, bytes_left, 0);
    if (n == -1) { break; }
    total += n;
    bytes_left -= n;
  }

  len = total;
  return n == -1 ? -1 : 0;
}


//sendEventToClient - builds a packet and sends it 
void ServerApplication::sendEventToClient(int client_fd, const GameEvent& event)
{
  PacketBuilder builder;
  builder.append8(static_cast<uint8_t>(event.type));
  builder.appendString(event.senderUUID.c_str());

  // ---> YOU MUST ADD THIS LINE <---
  builder.appendString(event.senderUsername.c_str()); 

  builder.append32(event.intPayload);
  builder.appendString(event.stringPayload.c_str());
  builder.finalize(); 

  
  size_t len = builder.getSize();

  ssize_t bytesSent = sendAll(client_fd, builder.getPtr(), len);
  if (bytesSent < 0) {
    std::cerr << "[NETWORK] Failed to send response to FD " << client_fd << "\n";
  }
}
