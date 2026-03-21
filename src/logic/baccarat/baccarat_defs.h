#ifndef BACCARAT_DEFS_H
#define BACCARAT_DEFS_H

#include "baccarat_hand.h"

enum class Baccarat_Outcome {
    BANKER = 0,
    PLAYER = 1,
    TIE = 2,
    PANDA = 3,
    DRAGON = 4
};


struct Round_Record {
  Baccarat_Outcome winner;
  int player_total;
  int banker_total;
  Baccarat_hand player_cards;
  Baccarat_hand banker_cards;
};

#endif
