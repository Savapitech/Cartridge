#include "games.h"
#include "../casino.h"

const game_t game_tab[] = {{"Menu    ", 4, menu},
                           {"Slots   ", 5, slot_machine},
                           {"Black Jack", 10, black_jack},
                           {"Roulette", 8, roulette},
                           {"Tax money", 9, tax_money}};

const uint8_t GAME_TAB_SIZE = 5;