#pragma once

#include <unordered_map>
#include <memory>
#include <functional>
#include <iostream>

#include "../../core/GameEvents.h"
#include "../../core/Debug.h"

#include "player.h"
// Include your TableManager so the SessionManager can talk to the pit boss
#include "TableManager.h" 

class SessionManager
{
public:
  using SendPacketFunc = std::function<void(int fd, const GameEvent& event)>;

  // 1. UPDATE CONSTRUCTOR: Pass in the TableManager by reference
  SessionManager(SendPacketFunc sendCallback, TableManager& tableManager) 
  : m_sendCallback(sendCallback), m_tableManager(tableManager) {}

  void onClientConnected(int client_fd)
  {
    DEBUG_PRINT << "SessionManager: onClientConnected():\n";
    DEBUG_PRINT << "  Preparing guest session for FD: " << client_fd << "\n";

    // Create a "Guest" player. They don't have a UUID or Username yet.
    // We use "Guest" as a placeholder until the SYS_Connect packet arrives.
    auto guestPlayer = std::make_unique<Player>(client_fd, "0000", "Guest");

    m_activePlayers[client_fd] = std::move(guestPlayer);
  }

  void handleClientMessage(int client_fd, const GameEvent& event)
  {
    DEBUG_PRINT << "SessionManager.h: handleClientMessage():\n";
    DEBUG_PRINT << "  event: " << static_cast<int>(event.type) << "\n";

    // ---------------------------------------------------------
    // SYSTEM EVENTS (Login, Disconnect, etc.)
    // ---------------------------------------------------------
    if (event.type == EventType::SYS_Connect)
    {
      DEBUG_PRINT << "    SYS_Connect: \n"
        << "    - Player logged in: " << event.senderUsername << "\n"
        << "    -  ID: " << event.senderUUID << "\n"
        << "    -  FD: " << client_fd << "\n";

      // Instantiating your actual Player class! 
      // Using your constructor: Player(int fd, std::string uuid, std::string username)
      auto newPlayer = std::make_unique<Player>(client_fd, event.senderUUID, event.senderUsername);

      // Store the player in our active session map
      m_activePlayers[client_fd] = std::move(newPlayer);

      GameEvent response;
      response.type = EventType::SYS_Connect_Success;
      m_sendCallback(client_fd, response);
      return; 
    }

    // ---------------------------------------------------------
    // TABLE MANAGER ROUTING (The Casino Floor)
    // ---------------------------------------------------------

    //authentication
    // If it's not a SYS_Connect, we need to find the actual Player object
    // to see who is trying to do something.
    auto it = m_activePlayers.find(client_fd);
    if (it == m_activePlayers.end()) {
      std::cerr << "Warning: Received event from unauthenticated FD: " << client_fd << "\n";
      return; // Drop the packet, they aren't logged in!
    }

    // Grab the raw pointer to the player
    Player* currentPlayer = it->second.get();

    // Now, route the event to the TableManager based on what they want to do
    // ---------------------------------------------------------
    // 3. MASTER ROUTING SWITCH
    // ---------------------------------------------------------
    switch (event.type) 
    {
      // === ACCOUNT & AUTHENTICATION ===
      case EventType::USER_In:
      case EventType::USER_Out:
      case EventType::USER_CreateAccount:
      case EventType::USER_ChangeUsername:
        // TODO: Hook up to database or auth manager
        DEBUG_PRINT << "Handling Auth Event for: " << currentPlayer->getUsername() << "\n";
        break;

      // === LOBBY & TABLE MANAGEMENT ===
      case EventType::GET_AvailableTables:
        {
          DEBUG_PRINT << "GET_AvailableTables: \n";
          GameEvent lobbyData = m_tableManager.getAvailableTablesListEvent();
          m_sendCallback(client_fd, lobbyData);
          break;
        }
      case EventType::CREATE_Table:
        {
          // TODO: Tell TableManager to spin up a new table dynamically
          break;
        }
      case EventType::JOIN_Table:
        {
          int tableToJoin = event.intPayload;
          bool success = m_tableManager.addPlayerToTable(tableToJoin, currentPlayer);
          if (success) { 
            // send success packet 
          } else { 
            // send table full error 
          }
          break;
        }
      case EventType::LEAVE_Table:
        {
          m_tableManager.removePlayerFromTable(currentPlayer->getCurrTableID(), currentPlayer);
          break;
        }

      // === CHAT ===
      case EventType::CHAT_Send:
        {
          // TODO: Broadcast chat message to the lobby or the specific table
          break;
        }

      // === IN-GAME ACTIONS (BACCARAT) ===
      case EventType::BC_USER_Bet:
        {
          m_tableManager.routeEventToTable(currentPlayer->getCurrTableID(), currentPlayer, event);
          break;
        }

      // === IN-GAME ACTIONS (BLACKJACK) ===
      case EventType::BJ_USER_PlaceBet:
      case EventType::BJ_USER_PlayerHit:
      case EventType::BJ_USER_PlayerStand:
        {
          m_tableManager.routeEventToTable(currentPlayer->getCurrTableID(), currentPlayer, event);
          break;
        }

      // === OUTBOUND/SERVER-ONLY EVENTS (Catch-all) ===
      // The server generates these, so if a client sends one, we ignore it.
      case EventType::SYS_Disconnect:
      case EventType::SYS_Connect_Success:
      case EventType::SYS_Connect_Failed:
      case EventType::NET_Connect_Success:
      case EventType::NET_Connect_Failed:
      case EventType::NET_NewAccount_Success:
      case EventType::JOIN_NET_Table:
      case EventType::LEAVE_NET_Table:
      case EventType::CHAT_NET_Recv:
      case EventType::BC_NET_Bet:
      case EventType::BC_NET_Ready:
      case EventType::BC_NET_AllReady:
        std::cerr << "[Warning] Client sent a server-only event type: " 
          << static_cast<int>(event.type) << "\n";
        break;

      default:
        std::cerr << "[Error] Unknown EventType received.\n";
        break;
    }
  }

  void onClientDisconnected(int client_fd)
  {
    DEBUG_PRINT << "SessionManager.h: onClientDisconnect(): Cleaning up data for disconnected FD: " << client_fd << "\n";

    auto it = m_activePlayers.find(client_fd);
    if (it != m_activePlayers.end())
    {
      Player* disconnectingPlayer = it->second.get();

      // [TABLE MANAGER HOOK]
      // Yank the player from whatever game they were playing so the table 
      // doesn't get stuck waiting for their turn.
      // m_tableManager.removePlayerFromAllTables(disconnectingPlayer);

      // Delete the player from memory
      m_activePlayers.erase(it);
    }
  }

private:
  SendPacketFunc m_sendCallback;

  // Reference to the Pit Boss
  TableManager& m_tableManager;

  // The actual roster of logged-in players mapped to their sockets
  std::unordered_map<int, std::unique_ptr<Player>> m_activePlayers; 
};
