#include <gb/gb.h>

#include "../casino.h"
#include "../utils/utils.h"
#include "games.h"
#include "menu.h"

#include "../assets/slot_asset.h"

const uint8_t slot_pos[4] = {40, 70, 100, 130};
uint8_t sprite_y[16];

void init_slot(void) {
  CLEAR_WIN;
  set_sprite_data(0, 1, star_tile);
  set_sprite_data(1, 1, cross_tile);

  for (uint8_t i = 0; i < 16; i++) {
    set_sprite_tile(i, 0);
    sprite_y[i] = 80 + ((i & 3) * 16);
    move_sprite(i, slot_pos[i >> 2], sprite_y[i]);
  }
  SHOW_SPRITES;
}

uint8_t slot_machine(bank_t *player_bank) {
  init_slot();
  draw_text(0, 0, player_bank->name);
  draw_money(player_bank->money, 0, 1);
  draw_text(4, 5, "SLOT MACHINE");

  draw_text(3, 9, "_______________");
  draw_text(3, 10, "|             |");
  draw_text(3, 11, "|_____________|");

  uint8_t keys = 0;
  uint8_t prev_keys = 0;
  uint8_t keys_pressed = 0;
  uint8_t stop_col = 0;

  uint8_t cooldown = 0;

  while (!(keys & J_SELECT)) {
    prev_keys = keys;
    keys = joypad();
    keys_pressed = (keys ^ prev_keys) & keys;

    if (cooldown > 0) {
      cooldown--;
    }

    if ((keys_pressed & J_B) && (cooldown == 0)) {
      if (stop_col < 16) {
        stop_col += 4;
        cooldown = 15;
      }
    }
    for (uint8_t i = stop_col; i < 16; i++) {
      sprite_y[i]++;

      if (sprite_y[i] >= 144) {
        sprite_y[i] = 80;
      }

      move_sprite(i, slot_pos[i >> 2], sprite_y[i]);
    }
    // wait_vbl_done();
  }

  return MENU;
}