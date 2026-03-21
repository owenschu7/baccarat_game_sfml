#include "baseTable.h"
#include "../network/server/player.h" // Ensure this path is correct for your Player class

BaseTable::BaseTable(int id, GameType t, int maxP) 
    : tableID(id), type(t), state(TableState::WAITING_FOR_PLAYERS), maxPlayers(maxP) 
{
}

bool BaseTable::addPlayer(Player* player) {
    // Basic validation: Is table full or game already running?
    if (currentPlayers.size() >= maxPlayers || state == TableState::GAME_IN_PROGRESS) {
        return false; 
    }
    
    currentPlayers.push_back(player);
    return true;
}

void BaseTable::removePlayer(Player* player) {
    // Remove the specific player pointer from the vector
    currentPlayers.erase(
        std::remove(currentPlayers.begin(), currentPlayers.end(), player),
        currentPlayers.end()
    );
    
    // If the table becomes empty, reset it to waiting
    if (currentPlayers.empty()) {
        state = TableState::WAITING_FOR_PLAYERS;
    }
}

// --- GETTER IMPLEMENTATIONS ---

int BaseTable::getTableID() const { 
    return tableID; 
}

GameType BaseTable::getGameType() const { 
    return type; 
}

TableState BaseTable::getState() const { 
    return state; 
}

int BaseTable::getPlayerCount() const { 
    return static_cast<int>(currentPlayers.size()); 
}

int BaseTable::getMaxPlayers() const { 
    return maxPlayers; 
}
