#ifndef PLAYER_H
#define PLAYER_H

#include <string>

using namespace std;

enum class BetChoice {
  None = 0,
  Player,
  Banker,
  Tie
};

class Player
{
public:
  Player();
  ~Player();
  
  // Fixed: Added initialization for m_totalWinnings and m_totalLosses to prevent garbage values
  Player(int fd, std::string uuid, std::string username) 
    : m_fd(fd), m_UUID(uuid), m_username(username), m_chips(1000), 
    m_totalWinnings(0), m_totalLosses(0), // <--- FIXED HERE
    m_currTableID(-1), m_currBetAmount(0), 
    m_currBetChoice(BetChoice::None), m_isReady(false), 
    m_isAuthenticated(false) {
  }

  // ----------------------------------------------------------------------
  // GETTERS (Read-only access for the Managers)
  // ----------------------------------------------------------------------
  int getFD() const;
  std::string getUUID() const;
  std::string getUsername() const;
  int getChips() const;
  int getCurrTableID() const;
  int getCurrBetAmount() const;
  BetChoice getCurrBetChoice() const;
  bool isReady() const;

  // ----------------------------------------------------------------------
  // SETTERS & ACTIONS (Safe ways to modify the player's state)
  // ----------------------------------------------------------------------
  void setCurrTableID(int tableID);
  void setReady(bool readyState);
  
  // Financial actions
  void setChips(int amount);
  void addChips(int amount);
  bool removeChips(int amount); // Returns false if they don't have enough
  
  // Betting actions
  bool placeBet(int amount, BetChoice choice);
  void clearBet();
  void recordGameResult(int amountWon, int amountLost);

private:
  // Identifiers
  std::string m_UUID;
  std::string m_username;
  int m_fd;

  // Financials
  int m_chips;

  // Stats
  int m_totalWinnings;
  int m_totalLosses;

  // Game state
  int m_currTableID;
  int m_currBetAmount;
  BetChoice m_currBetChoice;
  bool m_isReady;

  // Authentication
  bool m_isAuthenticated;
};

#endif
