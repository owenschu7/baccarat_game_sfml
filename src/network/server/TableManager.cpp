#include "TableManager.h"
#include "player.h"
#include <iostream>

TableManager::TableManager() {
  initializeDefaultTables();
}

void TableManager::initializeDefaultTables() {
  // Let's create two Baccarat tables to start.
  // Using your constructor: Baccarat_table(int id, int maxP)
  m_tables[1] = std::make_unique<Baccarat_table>(1, 8); 
  m_tables[2] = std::make_unique<Baccarat_table>(2, 8);

  DEBUG_PRINT << "TableManager: Initialized " << m_tables.size() << " tables on the casino floor.\n";
}

void TableManager::updateAll() {
  // Tick every single table. If a table is empty, its update() function 
  // will just instantly return, costing you basically zero CPU time.
  for (auto& pair : m_tables) {
    pair.second->update(); 
  }
}

void TableManager::routeEventToTable(int tableID, Player* player, const GameEvent& event) {
  auto it = m_tables.find(tableID);

  if (it != m_tables.end()) {
    // We found the table! Hand the event to the game logic.
    it->second->handleEvent(player, event);
  } else {
    std::cerr << "[Warning] Player " << player->getUsername() 
      << " sent an event to an invalid Table ID: " << tableID << "\n";
  }
}

//returns a GameEvent that holds
GameEvent TableManager::getAvailableTablesListEvent() {
  GameEvent event;

  // Updated to match your specific Enum name
  event.type = EventType::GET_AvailableTables; 

  // We use a stringstream to build a "CSV-style" string of all tables
  // Format: "ID,Type,Current,Max|ID,Type,Current,Max|"
  std::stringstream ss;

  for (const auto& pair : m_tables) {
    BaseTable* t = pair.second.get();

    ss << t->getTableID() << ","
      << static_cast<int>(t->getGameType()) << ","
      << t->getPlayerCount() << ","
      << t->getMaxPlayers() << "|";
  }

  // Store the built string in the payload
  event.stringPayload = ss.str();

  DEBUG_PRINT << "TableManager: Generated lobby list: " << event.stringPayload << "\n";

  return event;
}

bool TableManager::addPlayerToTable(int tableID, Player* player) {
  auto it = m_tables.find(tableID);
  if (it != m_tables.end()) {
    bool success = it->second->addPlayer(player);
    if (success) {
      // Update the player's internal state so they know where they are sitting
      player->setCurrTableID(tableID); 
      DEBUG_PRINT << "Player " << player->getUsername() << " joined Table " << tableID << "\n";
    }
    return success;
  }
  return false;
}

void TableManager::removePlayerFromTable(int tableID, Player* player) {
  auto it = m_tables.find(tableID);
  if (it != m_tables.end()) {
    it->second->removePlayer(player);
    player->setCurrTableID(-1); // -1 means "in the lobby"
  }
}

void TableManager::removePlayerFromAllTables(Player* player) {
  // If we already know what table they are at, we can just remove them from that one
  int currentTable = player->getCurrTableID();
  if (currentTable != -1) {
    removePlayerFromTable(currentTable, player);
  } else {
    // Fallback: search all tables just in case their state got out of sync
    for (auto& pair : m_tables) {
      pair.second->removePlayer(player);
    }
  }
}
