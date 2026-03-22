//ServerApplication class handles all data coming into the server
// - run() starts the application, gets data from either a new connection, or a existing user
// - each time a user sends data it strips the packet down into a game event and sends it to the session manager

#pragma once

#include <vector>
#include <unordered_map>
#include <poll.h> // for struct pollfd
#include <cstdint>

#include "../../core/GameEvents.h"
#include "SessionManager.h"

class ServerApplication
{
public:
  //Sets up the server and prepares it to listen on the given port
  ServerApplication(const char* port);
  ~ServerApplication();

  void run();

  //Breaks the infinite loop in run(), allowing the server to shut down gracefully.
  void stop();

  //sends a packet (thats made using PacketBuilder.h) through the socket
  //handles partial sends
  int sendAll(int sockfd, const uint8_t *buf, size_t &len);

private:
  bool setupServer();

  void acceptNewClient();

  // Reads raw incoming bytes from a specific player, puts them in their personal buffer,
  // and slices those bytes into complete 'GameEvents' (The Framing Loop).
  void handleClientData(size_t pollIndex);

  void disconnectClient(size_t pollIndex);

  // A helper function that takes a high-level GameEvent, packs it into a binary packet 
  // using PacketBuilder, and shoots it over the network using sendAll().
  void sendEventToClient(int client_fd, const GameEvent& event);

  const char* m_port;
  int m_serverSocket; //The master socket ID that the server uses to listen for BRAND NEW connections.
  bool m_isRunning;

  // A list of every active connection (including the master server socket).
  // The OS 'poll()' function uses this list to tell us who has data waiting to be read.
  // vector of pollfd structs
  std::vector<struct pollfd> m_pollFds;

  // The "Waiting Rooms". Maps a player's socket ID (int) to their personal stream of incoming bytes.
  // This prevents Player A's half-finished packets from mixing with Player B's packets.
  std::unordered_map<int, std::vector<uint8_t>> m_clientBuffers;

  // Owns all the casino tables, manages seating, and routes game events to the right table.
  TableManager m_tableManager;

  // Acts as the bridge between raw network sockets and actual "Players". 
  // Keeps track of who is logged in and what table they are currently sitting at.
  SessionManager m_sessionManager;
};
