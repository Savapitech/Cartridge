#include <gb/gb.h>
#include <rand.h>

#include "../casino.h"
#include "../utils/utils.h"
#include "games.h"
#include "menu.h"
#include "slot.h"

#include "../assets/slot_asset.h"

const uint8_t slot_pos[4] = {58, 77, 94, 110};
slot_t slots_array[16];

const uint8_t slot_machine_border[] = {
    T_TL, T_HZ, T_HZ, T_HZ, T_HZ, T_HZ, T_HZ, T_HZ, T_HZ, T_TR,
    T_VT, T_SP, T_SP, T_SP, T_SP, T_SP, T_SP, T_SP, T_SP, T_VT,
    T_BL, T_HZ, T_HZ, T_HZ, T_HZ, T_HZ, T_HZ, T_HZ, T_HZ, T_BR};

void init_slot(void) {
  CLEAR_WIN;
  set_sprite_data(0, 1, star_tile);
  set_sprite_data(1, 1, cross_tile);
  set_sprite_data(2, 1, circle_tile);
  set_sprite_data(3, 1, epitech_tile);

  slot_t *ptr = slots_array;
  for (uint8_t i = 0; i < 16; i++) {
    set_sprite_tile(i, 0);
    ptr->x = slot_pos[i >> 2];
    ptr->y = 80 + ((i & 3) << 4);
    ptr->type = (uint8_t)rand() & 3;
    set_sprite_tile(i, ptr->type);
    move_sprite(i, ptr->x, ptr->y);
    ptr++;
  }

  draw_text(4, 5, "SLOT MACHINE");
  set_win_tiles(5, 9, 10, 3, slot_machine_border);

  SHOW_SPRITES;
}

uint8_t check_win(void) {
  uint8_t winning_types[4];
  uint8_t found = 0;

  for (uint8_t i = 0; i < 16; i++) {
    if (slots_array[i].y == TARGET_Y) {
      winning_types[found] = slots_array[i].type;
      found++;
    }
  }

  if (found != 4)
    return LOOSE;

  if (winning_types[0] == winning_types[1] &&
      winning_types[1] == winning_types[2] &&
      winning_types[2] == winning_types[3]) {
    return WIN;
  }
  return LOOSE;
}

void stop_slot(uint8_t col_start) {
  slot_t *col = &slots_array[col_start];
  uint8_t best_idx = 0;
  uint8_t min_dist = 255;
  for (uint8_t i = 0; i < 4; i++) {
    uint8_t dist =
        (col[i].y > TARGET_Y) ? (col[i].y - TARGET_Y) : (TARGET_Y - col[i].y);

    if (dist < min_dist) {
      min_dist = dist;
      best_idx = i;
    }
  }
  while (col[best_idx].y != TARGET_Y) {
    int8_t step = (col[best_idx].y < TARGET_Y) ? 1 : -1;

    for (uint8_t i = 0; i < 4; i++) {
      col[i].y += step;
      if (col[i].y >= 144)
        col[i].y = 80;
      else if (col[i].y < 80)
        col[i].y = 143;

      move_sprite(col_start + i, col[i].x, col[i].y);
    }
    wait_vbl_done();
  }
  for (uint8_t i = 0; i < 4; i++) {
    if (i != best_idx) {
      col[i].y = 0;
      move_sprite(col_start + i, 0, 0);
    }
  }
}

uint8_t slot_machine(bank_t *player_bank) {
  init_slot();
  draw_text(0, 0, player_bank->name);
  draw_money(player_bank->money, 0, 1);

  uint8_t keys = 0;
  uint8_t prev_keys = 0;
  uint8_t keys_pressed = 0;
  uint8_t stop_col = 0;
  uint8_t cooldown = 0;

  while (!(keys & J_SELECT)) {
    prev_keys = keys;
    keys = joypad();
    keys_pressed = (keys ^ prev_keys) & keys;

    if (cooldown > 0)
      cooldown--;

    if (keys & J_A)
      return SLOT;

    if ((keys_pressed & J_B) && (cooldown == 0)) {
      if (stop_col < 16) {
        stop_slot(stop_col);
        stop_col += 4;
        cooldown = 15;
      }
      if (stop_col == 16) {
        if (check_win()) {
          player_bank->money += 100;
          draw_money(player_bank->money, 0, 1);
        }
      }
    }

    slot_t *ptr = slots_array + stop_col;

    for (uint8_t i = stop_col; i < 16; i++) {
      ptr->y += 3;
      if (ptr->y >= 144) {
        ptr->y = 80;
      }
      move_sprite(i, ptr->x, ptr->y);
      ptr++;
    }
    wait_vbl_done();
  }

  return MENU;
}