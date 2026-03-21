#pragma once

#include <unordered_map>
#include <memory>
#include <vector>
#include <string>
#include <sstream>

#include "../../core/GameEvents.h"
#include "../../core/Debug.h"

// Bring in the game logic
#include "../../logic/baseTable.h"
#include "../../logic/baccarat/baccarat_table.h"
// If you add Poker or Blackjack later, include them here!

class Player; // Forward declaration

class TableManager {
private:
    // The master list of all active tables on the server.
    // Maps TableID -> The actual Table object
    std::unordered_map<int, std::unique_ptr<BaseTable>> m_tables;

public:
    TableManager();
    ~TableManager() = default;

    // Sets up the initial casino floor when the server boots
    void initializeDefaultTables();

    // The Global Tick: Called by ServerApplication ~10 times a second
    void updateAll();

    // ------------------------------------------------------------------
    // EVENT ROUTING
    // ------------------------------------------------------------------
    
    // Takes an action from SessionManager and hands it to the specific table
    void routeEventToTable(int tableID, Player* player, const GameEvent& event);

    // Packages up all the table info so SessionManager can send it to a new client
    GameEvent getAvailableTablesListEvent();

    // ------------------------------------------------------------------
    // PLAYER MANAGEMENT
    // ------------------------------------------------------------------
    
    bool addPlayerToTable(int tableID, Player* player);
    void removePlayerFromTable(int tableID, Player* player);
    
    // Crucial for when a player's network cable gets unplugged
    void removePlayerFromAllTables(Player* player); 
};
