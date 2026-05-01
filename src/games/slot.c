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

void init_slot(void) 
{
  CLEAR_BKG;
  NR52_REG = 0x80;
  NR51_REG = 0xFF;
  NR50_REG = 0x77;
  set_sprite_data(0, 1, star_tile);
  set_sprite_data(1, 1, cross_tile);
  set_sprite_data(2, 1, circle_tile);
  set_sprite_data(3, 1, epitech_tile);

  slot_t *ptr = slots_array;
  for (uint8_t i = 0; i < 16; i++) {
    set_sprite_tile(i, 0);
    ptr->x = slot_pos[i >> 2];
    ptr->y = 80 + ((i & 3) << 4);
    ptr->type = 0;
    ptr->type = (uint8_t)rand() & 3;
    set_sprite_tile(i, ptr->type);
    if (ptr->y < TARGET_Y - 2 || ptr->y > TARGET_Y + 4) {
      move_sprite(i, 0, 0);
    } else {
      move_sprite(i, ptr->x, ptr->y);
    }
    ptr++;
  }

  draw_text(4, 5, "SLOT MACHINE");
  set_bkg_tiles(5, 9, 10, 3, slot_machine_border);
  set_bkg_tiles(5, 9, 10, 3, slot_machine_border);
  SHOW_SPRITES;
}

inline void play_jackpot(void)
{
    NR21_REG = 0x80;
    NR22_REG = 0xF3;
    NR23_REG = 0x40;
    NR24_REG = 0xC3;
    delay(100);

    NR23_REG = 0x70;
    NR24_REG = 0xC3;
    delay(100);
}

inline void play_loose(void)
{
    NR21_REG = 0x80;
    NR22_REG = 0xF2;

    NR23_REG = 0x70;
    NR24_REG = 0xC3;
    delay(80);

    NR23_REG = 0x50;
    NR24_REG = 0xC3;
    delay(80);

    NR23_REG = 0x30;
    NR24_REG = 0xC3;
    delay(120);
}

void win_animation()
{
  uint8_t y = 0;
  uint8_t coin_x[5] = {20, 50, 80, 110, 140};
  uint8_t coin_y[5] = {0, 220, 240, 210, 230};

  for (uint8_t i = 0; i < 5; i++) {
    set_sprite_tile(i, 2);
  }

  while (y != 100)
  {
    if ((y >> 3) & 1) {
      HIDE_SPRITES;
      HIDE_BKG;
    } else {
      SHOW_SPRITES;
      SHOW_BKG;
    }

    for (uint8_t i = 0; i < 5; i++) {
      coin_y[i] += 4;
      if (coin_y[i] > 160 && coin_y[i] < 200)
        coin_y[i] = 0;
      move_sprite(i, coin_x[i], coin_y[i]);
    }

    wait_vbl_done();

    if ((y & 15) == 0) {
      NR21_REG = 0x80;
      NR22_REG = 0x82;
      NR23_REG = 0x80;
      NR24_REG = 0xC7;
    }

    y++;
  }
  SHOW_SPRITES;
  SHOW_BKG;
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
    play_jackpot();
    return WIN;
  }
  play_loose();
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
      if (col[i].y < TARGET_Y - 2 || col[i].y > TARGET_Y + 4) {
        move_sprite(col_start + i, 0, 0);
      } else {
        move_sprite(col_start + i, col[i].x, col[i].y);
      }
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
  uint8_t target_type = 255;

  while (!(keys & J_SELECT)) 
  {
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
        if (stop_col == 0) {
          for (uint8_t i = 0; i < 4; i++) {
            if (slots_array[i].y == TARGET_Y) {
              target_type = slots_array[i].type;
              break;
            }
          }
        }
        stop_col += 4;
        cooldown = 15;
      }

      if (stop_col == 16) {
        stop_col = 0;
        target_type = 255;
        delay(500);
        for (slot_t *col = &slots_array[0]; stop_col < 16; stop_col++){
          move_sprite(stop_col, 200, 200);
          col++;
        }
        if (check_win()) {
          player_bank->money += 100;
          draw_money(player_bank->money, 0, 1);
          win_animation();
        }
        return SLOT;
      }
    }

    slot_t *ptr = slots_array + stop_col;

    for (uint8_t i = stop_col; i < 16; i++) {
      ptr->y += 2;
      if (ptr->y >= 144) {
        if (target_type != 255 && ((uint8_t)rand() & 1) == 0) {
            ptr->type = target_type;
        } else {
            ptr->type = (uint8_t)rand() & 3;
        }
        set_sprite_tile(i, ptr->type);
        ptr->y = 80;
      }

      if (ptr->y < TARGET_Y - 2 || ptr->y > TARGET_Y + 4) {
        move_sprite(i, 0, 0);
      } else {
        move_sprite(i, ptr->x, ptr->y);
      }
      ptr++;
    }
    wait_vbl_done();
  }
  return MENU;
}