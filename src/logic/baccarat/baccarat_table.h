//implements the ability to play a round of baccarat and stores the data of all the previous games
#ifndef BACCARAT_TABLE_H
#define BACCARAT_TABLE_H

#include <vector>
#include "../deck.h"
#include "../../network/server/player.h"
#include "baccarat_defs.h"
#include "../baseTable.h"

using namespace std;

class Baccarat_table: public BaseTable
{
public:
  Baccarat_table(int id, int max_players);
  ~Baccarat_table();

  //plays a round returns who wins also it pushes the result to m_outcomes
  Round_Record play_round(); // plays 1 round and returns a record of that round played
 
  void print_history_of_round(int round);

  void handleEvent(Player* player, const GameEvent& event) override;
  void update() override;

private:
  Deck m_deck{8}; // 8 decks in a shoe
  int m_cards_left;
  vector<Round_Record> m_outcomes; // stores all the information about the game
};

#endif
