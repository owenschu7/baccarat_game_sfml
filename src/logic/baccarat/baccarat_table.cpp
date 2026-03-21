#include <iostream>
#include "../card.h"
#include "../deck.h"
#include "baccarat_hand.h"
#include "baccarat_table.h"
#include "baccarat_defs.h"

Baccarat_table::Baccarat_table(int id, int maxP) : BaseTable(id, GameType::BACCARAT, maxP)
{
}

Baccarat_table::~Baccarat_table()
{
}

// TEST: this code is for testing baccarat functionality
void Baccarat_table::print_history_of_round(int round)
{
  cout << "=========Player========= total = " << m_outcomes[round-1].player_total << endl;
  m_outcomes[round-1].player_cards.get_first_card().print_card();
  m_outcomes[round-1].player_cards.get_second_card().print_card();
  m_outcomes[round-1].player_cards.get_third_card().print_card();
  cout << "=========Banker========= total = " << m_outcomes[round-1].banker_total << endl;
  m_outcomes[round-1].banker_cards.get_first_card().print_card();
  m_outcomes[round-1].banker_cards.get_second_card().print_card();
  m_outcomes[round-1].banker_cards.get_third_card().print_card();
}

Round_Record Baccarat_table::play_round()
{
  // start the round
  m_deck.shuffle_deck(); //TODO: shoe should be shuffled only when shoe ends
  if (m_cards_left < 10)
  {
    cout << "Reshuffling Shoe..." << endl;
    
    m_outcomes.clear();

    // new deck
    m_deck = Deck(8); 
    
    //shuffle the deck
    m_deck.shuffle_deck();
  }
  //play a round

  Round_Record current_round;

  Baccarat_hand player(m_deck.deal_card(), m_deck.deal_card());
  Baccarat_hand banker(m_deck.deal_card(), m_deck.deal_card());

  int player_total = player.get_total();
  int banker_total = banker.get_total();

  Baccarat_Outcome winner;
  //check for naturals
  if (player_total >= 8 || banker_total >= 8)
  {
    // Determine winner logic 
    if(player_total == banker_total)
    {
      //tie
      winner = Baccarat_Outcome::TIE;
      
    } else if (player_total > banker_total)
    {
      //player win
      winner = Baccarat_Outcome::PLAYER;
    } else {
      //banker win
      winner = Baccarat_Outcome::BANKER;
    }
    //append cards to outcome
    current_round.banker_cards = banker; // (of type Baccarat_hand)
    current_round.player_cards = player;
    current_round.banker_total = banker_total;
    current_round.player_total = player_total;
    current_round.winner = winner;
    m_outcomes.push_back(current_round);

    return current_round;
  }

  //no naturals occured
  //continue...

  int player_third_card_value = -1; // flag to track if player drew (-1 means no draw)

  // 3. player's third card logic
  // player draws if total is 0-5. stands on 6-7.
  bool player_draws = false;
  if (player_total <= 5 && banker_total < 8) //player draws if total<=5 and natural did not happen
  {
    Card c = m_deck.deal_card();
    player_third_card_value = c.calc_card_value(); // assuming get_value() returns baccarat value (0 for 10/j/q/k)
    player.add_third_card(c);
    player_draws = true;
  }

  // 4. banker's third card logic
  // banker's move depends on whether player drew a card
  bool banker_draws = false;

  if (player_third_card_value == -1)
  {
    // scenario a: player stood (did not draw)
    // banker draws if 0-5, stands if 6-7
    if (banker_total <= 5) {
      banker_draws = true;
    }
  } else {
    // scenario b: player drew a third card
    // banker acts based on own score and player's third card value
    switch (banker_total)
    {
      case 0: case 1: case 2:
        banker_draws = true; // always draw
        break;
      case 3:
        // draw unless player's third card was an 8
        if (player_third_card_value != 8) banker_draws = true;
        break;
      case 4:
        // draw if player's third card was 2, 3, 4, 5, 6, 7
        if (player_third_card_value >= 2 && player_third_card_value <= 7) banker_draws = true;
        break;
      case 5:
        // draw if player's third card was 4, 5, 6, 7
        if (player_third_card_value >= 4 && player_third_card_value <= 7) banker_draws = true;
        break;
      case 6:
        // draw if player's third card was 6 or 7
        if (player_third_card_value == 6 || player_third_card_value == 7) banker_draws = true;
        break;
      case 7:
        banker_draws = false; // always stand
        break;
    }
  }

  if (banker_draws) {
      banker.add_third_card(m_deck.deal_card());
  }

  // 5. final calculation
  player_total = player.get_total();
  banker_total = banker.get_total();

  if (player_total == banker_total)
  {
        winner = Baccarat_Outcome::TIE;
  } else if (banker_total > player_total)
  {
    if (banker_total == 7 && banker_draws)
    {
      winner = Baccarat_Outcome::DRAGON;
    } else {
      winner = Baccarat_Outcome::BANKER;
    }
  } else {
    if (player_total == 8 && player_draws)
    {
      winner = Baccarat_Outcome::PANDA;
    } else {
      winner = Baccarat_Outcome::PLAYER;
    }
  }

  //append cards to Round_Record
  current_round.banker_cards = banker; // (of type Baccarat_hand)
  current_round.player_cards = player;
  current_round.winner = winner;
  current_round.banker_total = banker_total;
  current_round.player_total = player_total;
  m_outcomes.push_back(current_round);

  return current_round;
}


//for base class
void Baccarat_table::handleEvent(Player* player, const GameEvent& event) {
    // This is where you will eventually check what the player sent over the network.
    // e.g., if (event.type == PLACE_BET) { record player's bet }
}

void Baccarat_table::update() {
    // A super simple, bare-bones way to run the game
    if (state == TableState::WAITING_FOR_PLAYERS) {
        if (currentPlayers.size() > 0) {
            state = TableState::GAME_IN_PROGRESS;
        }
    } 
    else if (state == TableState::GAME_IN_PROGRESS) {
        
        // BOOM. Here is where you use your old logic exactly as it was.
        // It plays the round, does the math, and returns the record.
        Round_Record result = play_round(); 

        // Now you just take 'result' and use it to figure out who won
        // so you can send the results to the network!
        
        state = TableState::WAITING_FOR_PLAYERS; // Reset for the next hand
    }
}
