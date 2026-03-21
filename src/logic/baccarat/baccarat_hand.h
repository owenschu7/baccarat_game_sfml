#ifndef BACCARAT_HAND
#define BACCARAT_HAND
#include "../card.h"

//this is a class for handling calculations of certain card combinations
//holds 2-3 card objects and can calculate total score
// NOTE: whether or not a thrid card will be dealt will be handled by different class, this class just holds the hand of either player or banker
//
class Baccarat_hand
{
public:

  Baccarat_hand();
  Baccarat_hand(Card first_card);
  Baccarat_hand(Card first_card, Card second_card);
  Baccarat_hand(Card first_card, Card second_card, Card third_card);
  ~Baccarat_hand();

  void add_second_card(Card second_card);
  void add_third_card(Card third_card);

  Card get_first_card();
  Card get_second_card();
  Card get_third_card();

  int calc_total();

  int get_total();


private:
  Card m_first_card;
  Card m_second_card;
  Card m_third_card;
  int m_total;
};

#endif
