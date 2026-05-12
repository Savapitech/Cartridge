#ifndef POKER_H
#define POKER_H

#define POKER_CARDS 5

#define HAND_NOTHING 0
#define HAND_JACKS_BETTER 1
#define HAND_TWO_PAIR 2
#define HAND_THREE_KIND 3
#define HAND_STRAIGHT 4
#define HAND_FLUSH 5
#define HAND_FULL_HOUSE 6
#define HAND_FOUR_KIND 7
#define HAND_STRAIGHT_FLUSH 8
#define HAND_ROYAL_FLUSH 9

#define CPOS(n) (1 + (n) * 3)

#define ROW_BET_TXT 1
#define ROW_CARD_TOP 3
#define ROW_CARD_BOT 4
#define ROW_HOLD 5
#define ROW_CURSOR 6
#define ROW_HAND 8
#define ROW_RESULT 9
#define ROW_HINT 11
#define ROW_PAY1 13
#define ROW_PAY2 14
#define ROW_PAY3 15

#endif
