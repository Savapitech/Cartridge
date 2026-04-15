#include <gb/gb.h>
#include <gbdk/console.h>
#include <gbdk/font.h>
#include <stdio.h>
#include <string.h>

#include "casino.h"
#include "games/games.h"
#include "utils/utils.h"

void init(void) {
  DISPLAY_ON;

  SHOW_BKG;
  SHOW_WIN;
  SHOW_SPRITES;

  BGP_REG = 0xE4;
  OBP0_REG = 0xD2;

  SCX_REG = 0;
  SCY_REG = 0;
}

void init_font(void) {
  font_init();

  font_t font = font_load(font_ibm);
  font_set(font);
}

void main(void) {
  bank_t bank = {.name = "Arthur", .name_size = 6, .money = 0};

  uint8_t game_idx = 0;

  init();
  init_font();

  draw_text(0, 0, bank.name);

  while (1) {
    render_money(&bank);
    game_idx = game_tab[game_idx].game(&bank);
    draw_money((uint32_t)game_idx, 0, 2);
    wait_vbl_done();
  }
}