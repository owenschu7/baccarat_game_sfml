/* =========================================================================
 * SessionManager
 * PURPOSE:
 * This class acts as the bridge between the raw networking layer
 * (ServerApplication) and the game logic layer (TableManager).
 * It does not handle sockets, it does not run card games.
 * It manages the PEOPLE.
 *
 * HOW IT FITS THE ARCHITECTURE:
 * 1.) ServerApplication gets raw bytes, frames a packet, and hands it here.
 * 2.) SessionManager figures out WHO sent it and WHAT they want to do.
 * - If they want to log in, SessionManager handles it.
 * - If they want to place a bet, SessionManager forwards it to TableManager.
 * 3.) If a game needs to send a message back to a player, it tells the
 * SessionManager which uses sendCallback to send a message back to the server.
 * ========================================================================= */

//SessionManager class
//  keeps track of all the players in the server and what they are currently doing
//  - game events are routed from the server into the table
#pragma once

#include <unordered_map>
#include <memory>
#include <functional>

#include "../../core/GameEvents.h"
#include "player.h"
#include "TableManager.h" 

class SessionManager
{
public:
  // create a namespace for a function
  using SendEventCallback = std::function<void(int fd, const GameEvent& event)>;

  // Constructor: Pass in the TableManager by reference
  SessionManager(SendEventCallback sendCallback, TableManager& tableManager);

  // Network Event Handlers
  void onClientConnected(int client_fd);
  void handleClientMessage(int client_fd, const GameEvent& event);
  void onClientDisconnected(int client_fd);

private:
  SendEventCallback m_sendCallback;

  // Reference to the Pit Boss
  TableManager& m_tableManager;

  // The actual roster of logged-in players mapped to their sockets
  std::unordered_map<int, std::unique_ptr<Player>> m_activePlayers; 
};
