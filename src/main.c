#include <gb/gb.h>
#include <stdio.h>
#include <string.h>

#include "init/init.h"
#include "casino.h"
#include "games/games.h"
#include "utils/utils.h"

void main(void) {
  bank_t bank = {.name = "Arthur", .name_size = 6, .money = 0};

  uint8_t game_idx = 0;

  init();
  init_font();

  draw_text(0, 0, bank.name);

  while (1) {
    game_idx = game_tab[game_idx].game(&bank);
    wait_vbl_done();
  }
}