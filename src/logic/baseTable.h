#pragma once
#include <vector>
#include <algorithm>
#include "../core/GameEvents.h" 

// Forward declaration of Player to avoid circular includes
class Player;

enum class GameType {
    BACCARAT = 1,
    POKER = 2,
    BLACKJACK = 3
};

enum class TableState {
    WAITING_FOR_PLAYERS,
    BETTING_PHASE,
    GAME_IN_PROGRESS,
    PAYOUT
};

class BaseTable {
protected:
    int tableID;
    GameType type;
    TableState state;
    int maxPlayers;
    std::vector<Player*> currentPlayers; 

public:
    BaseTable(int id, GameType t, int maxP);
    virtual ~BaseTable() = default; 

    // --- PURE VIRTUAL FUNCTIONS ---
    virtual void handleEvent(Player* player, const GameEvent& event) = 0;
    virtual void update() = 0; 

    // --- COMMON ACTIONS ---
    virtual bool addPlayer(Player* player);
    virtual void removePlayer(Player* player);

    // --- GETTERS (Needed by TableManager) ---
    int getTableID() const;
    GameType getGameType() const;
    TableState getState() const;
    int getPlayerCount() const;
    int getMaxPlayers() const;
};
