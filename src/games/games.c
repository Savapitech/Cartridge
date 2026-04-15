#include "games.h"
#include "../casino.h"

#define GAME_NAME(a) a, sizeof(a)

const game_t game_tab[] = {{GAME_NAME("Menu    "), menu},
                           {GAME_NAME("Slots   "), slot_machine},
                           {GAME_NAME("Black Jack"), black_jack},
                           {GAME_NAME("Roulette"), roulette},
                           {GAME_NAME("Tax money"), tax_money},
                           {GAME_NAME("Cheat code"), cheat_code}};

const uint8_t GAME_TAB_SIZE = 6;