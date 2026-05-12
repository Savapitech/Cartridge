#include <gb/gb.h>

#include "casino.h"
#include "games/games.h"
#include "init/init.h"
#include "save/save.h"
#include "utils/utils.h"

uint8_t luck;

void main(void) {
  bank_t bank = {.name = "Player", .name_size = 6, .money = 0};

  uint8_t game_idx = 0;

  init();
  init_font();

  save_load(bank.name, &bank);

  draw_text(0, 0, bank.name);

  for (;;) {
    game_idx = game_tab[game_idx].game(&bank);
    save_write(&bank);
    wait_vbl_done();
  }
}
