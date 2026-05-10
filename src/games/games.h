#ifndef GAMES_H
#define GAMES_H

#include <stdint.h>

#include "../casino.h"

uint8_t black_jack(bank_t *player_bank);
uint8_t slot_machine(bank_t *player_bank);
uint8_t roulette(bank_t *player_bank);
uint8_t menu(bank_t *player_bank);
uint8_t tax_money(bank_t *player_bank);
uint8_t cheat_code(bank_t *player_bank);

typedef struct game_s {
  char name[16];
  uint8_t name_size;
  uint8_t (*game)(bank_t *);
} game_t;

#define MENU 0
#define SLOT MENU + 1
#define BLAC_JACK SLOT + 1
#define ROULETTE BLAC_JACK + 1
#define TAX_MONEY ROULETTE + 1
#define CHEAT_CODE TAX_MONEY + 1

extern const game_t game_tab[];
extern const uint8_t GAME_TAB_SIZE;

#endif