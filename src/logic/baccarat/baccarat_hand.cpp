#include "../card.h"
#include "baccarat_hand.h"

Baccarat_hand::Baccarat_hand()
{
    //initialize default constructor
    m_first_card = Card();
    m_second_card = Card();
    m_third_card = Card();
    m_total = -1;
}

Baccarat_hand::Baccarat_hand(Card first_card)
{
    m_first_card = first_card;
    m_second_card = Card();
    m_third_card = Card();
    m_total = calc_total();
}

Baccarat_hand::Baccarat_hand(Card first_card, Card second_card)
{
    m_first_card = first_card;
    m_second_card = second_card;
    m_third_card = Card();
    m_total = calc_total();
}

Baccarat_hand::Baccarat_hand(Card first_card, Card second_card, Card third_card)
{
    m_first_card = first_card;
    m_second_card = second_card;
    m_third_card = third_card;
    m_total = calc_total();
}

Baccarat_hand::~Baccarat_hand()
{
}

void Baccarat_hand::add_second_card(Card second_card)
{
    m_second_card = second_card;
    m_total = calc_total();
}

void Baccarat_hand::add_third_card(Card third_card)
{
    m_third_card = third_card;
    m_total = calc_total();
}

Card Baccarat_hand::get_first_card()
{
    return m_first_card;
}

Card Baccarat_hand::get_second_card()
{
    return m_second_card;
}

Card Baccarat_hand::get_third_card()
{
    return m_third_card;
}

int Baccarat_hand::calc_total()
{
    //add all three cards point values together 2-9 = 2-9; 10 = 0; jack(11), queen(12), king(13) = 0; ace(14) = 1
    //if a card hasnt been added to the class yet, it will default to 0
    return (m_first_card.calc_card_value() + m_second_card.calc_card_value() + m_third_card.calc_card_value()) % 10;
}

int Baccarat_hand::get_total()
{
    return m_total;
}
