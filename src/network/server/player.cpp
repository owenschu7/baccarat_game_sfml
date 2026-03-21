#include "player.h"

// Default constructor (In case you ever need to create a blank player)
Player::Player() 
    : m_fd(-1), m_chips(0), m_totalWinnings(0), m_totalLosses(0),
      m_currTableID(-1), m_currBetAmount(0), m_currBetChoice(BetChoice::None),
      m_isReady(false), m_isAuthenticated(false) 
{
}

// Destructor
Player::~Player() 
{
    // Nothing dynamically allocated (no raw pointers), so this can be empty!
}

// --- GETTERS ---

int Player::getFD() const { return m_fd; }
std::string Player::getUUID() const { return m_UUID; }
std::string Player::getUsername() const { return m_username; }
int Player::getChips() const { return m_chips; }
int Player::getCurrTableID() const { return m_currTableID; }
int Player::getCurrBetAmount() const { return m_currBetAmount; }
BetChoice Player::getCurrBetChoice() const { return m_currBetChoice; }
bool Player::isReady() const { return m_isReady; }

// --- SETTERS & ACTIONS ---

void Player::setCurrTableID(int tableID) 
{
    m_currTableID = tableID;
    
    // If they leave a table (tableID == -1), clear their active bet
    if (tableID == -1) {
        clearBet();
        m_isReady = false;
    }
}

void Player::setReady(bool readyState) 
{
    m_isReady = readyState;
}

void Player::setChips(int amount) 
{
    m_chips = amount;
}

void Player::addChips(int amount) 
{
    if (amount > 0) {
        m_chips += amount;
    }
}

bool Player::removeChips(int amount) 
{
    if (amount > 0 && m_chips >= amount) {
        m_chips -= amount;
        return true;
    }
    return false; // Not enough chips!
}

bool Player::placeBet(int amount, BetChoice choice) 
{
    // Make sure they aren't already betting, and that they have the money
    if (m_currBetAmount == 0 && removeChips(amount)) {
        m_currBetAmount = amount;
        m_currBetChoice = choice;
        return true;
    }
    return false; 
}

void Player::clearBet() 
{
    m_currBetAmount = 0;
    m_currBetChoice = BetChoice::None;
}

void Player::recordGameResult(int amountWon, int amountLost)
{
    m_totalWinnings += amountWon;
    m_totalLosses += amountLost;
    
    if (amountWon > 0) {
        addChips(amountWon);
    }
    
    clearBet(); // Reset for the next round
}
