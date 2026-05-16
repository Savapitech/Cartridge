#include <gb/gb.h>
#include <rand.h>

#include "../assets/button.h"
#include "../assets/slotBorder.h"
#include "../assets/slot_asset.h"
#include "../audio/audio.h"
#include "../casino.h"
#include "../utils/transitions.h"
#include "../utils/utils.h"
#include "games.h"
#include "menu.h"
#include "slot.h"

const uint8_t slot_pos[4] = {58, 77, 94, 110};
const uint8_t bet_options[4] = {5, 10, 25, 50};
slot_t slots_array[16];

static const uint8_t win_brd_tl[16] = {0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xC0,
                                       0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0,
                                       0xC0, 0xC0, 0xC0, 0xC0};
static const uint8_t win_brd_tr[16] = {0xFF, 0xFF, 0xFF, 0xFF, 0x03, 0x03,
                                       0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
                                       0x03, 0x03, 0x03, 0x03};
static const uint8_t win_brd_bl[16] = {0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0,
                                       0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0,
                                       0xFF, 0xFF, 0xFF, 0xFF};
static const uint8_t win_brd_br[16] = {0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
                                       0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
                                       0xFF, 0xFF, 0xFF, 0xFF};
static const uint8_t win_brd_h[16] = {0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00};
static const uint8_t win_brd_hb[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                       0xFF, 0xFF, 0xFF, 0xFF};
static const uint8_t win_brd_vl[16] = {0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0,
                                       0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0,
                                       0xC0, 0xC0, 0xC0, 0xC0};
static const uint8_t win_brd_vr[16] = {0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
                                       0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
                                       0x03, 0x03, 0x03, 0x03};

static const uint8_t slot_machine_border[30] = {
    SLOT_BRD_TL, SLOT_BRD_H,  SLOT_BRD_H,  SLOT_BRD_H,  SLOT_BRD_H,
    SLOT_BRD_H,  SLOT_BRD_H,  SLOT_BRD_H,  SLOT_BRD_H,  SLOT_BRD_TR,
    SLOT_BRD_VL, 0,           0,           0,           0,
    0,           0,           0,           0,           SLOT_BRD_VR,
    SLOT_BRD_BL, SLOT_BRD_HB, SLOT_BRD_HB, SLOT_BRD_HB, SLOT_BRD_HB,
    SLOT_BRD_HB, SLOT_BRD_HB, SLOT_BRD_HB, SLOT_BRD_HB, SLOT_BRD_BR};

static void play_jackpot(void) {
  uint8_t i;
  NR21_REG = 0x80;
  NR22_REG = 0xF3;
  NR23_REG = 0x40;
  NR24_REG = 0xC3;
  delay(90);
  NR23_REG = 0x70;
  NR24_REG = 0xC3;
  delay(90);
  NR23_REG = 0x90;
  NR24_REG = 0xC3;
  delay(90);
  for (i = 0; i < 3; i++) {
    NR21_REG = 0x82;
    NR22_REG = 0xF1;
    NR23_REG = 0x55 + i * 0x10;
    NR24_REG = 0xC3;
    delay(60);
  }
  NR21_REG = 0x80;
  NR22_REG = 0xF1;
  NR23_REG = 0xA0;
  NR24_REG = 0xC5;
  delay(300);
}

static void play_loose(void) {
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

static void play_spin_tick(void) {
  NR41_REG = 0x3F;
  NR42_REG = 0x51;
  NR43_REG = 0x28;
  NR44_REG = 0x80;
}

static void play_stop_clack(void) {
  NR41_REG = 0x20;
  NR42_REG = 0x72;
  NR43_REG = 0x16;
  NR44_REG = 0x80;
  delay(40);
  NR41_REG = 0x20;
  NR42_REG = 0x61;
  NR43_REG = 0x26;
  NR44_REG = 0x80;
}

static void play_near_miss(void) {
  NR21_REG = 0x80;
  NR22_REG = 0xB2;
  NR23_REG = 0x60;
  NR24_REG = 0xC3;
  delay(120);
  NR21_REG = 0x80;
  NR22_REG = 0x82;
  NR23_REG = 0x45;
  NR24_REG = 0xC3;
  delay(150);
}

static void win_animation(void) {
  uint8_t y = 0;
  uint8_t coin_x[5] = {20, 50, 80, 110, 140};
  uint8_t coin_y[5] = {0, 220, 240, 210, 230};
  uint8_t i;

  for (i = 0; i < 5; i++)
    set_sprite_tile(i, (uint8_t)(SLOT_SPR_BASE + 2));

  play_jackpot();

  while (y != 120) {
    if ((y >> 2) & 1) {
      HIDE_SPRITES;
      HIDE_BKG;
    } else {
      SHOW_SPRITES;
      SHOW_BKG;
    }

    for (i = 0; i < 5; i++) {
      coin_y[i] += 4;
      if (coin_y[i] > 160 && coin_y[i] < 200)
        coin_y[i] = 0;
      move_sprite(i, coin_x[i], coin_y[i]);
    }

    if ((y & 15) == 0) {
      NR21_REG = 0x80;
      NR22_REG = 0x82;
      NR23_REG = 0x80;
      NR24_REG = 0xC7;
    }

    wait_vbl_done();
    y++;
  }
  SHOW_SPRITES;
  SHOW_BKG;
}

static uint8_t choose_bet_slot(bank_t *player_bank) {
  uint8_t selected = 1;
  uint8_t keys = 0, prev_keys, keys_pressed;
  uint8_t i;

  {
    uint8_t s;
    for (s = 0; s < 40; s++)
      move_sprite(s, 0, 0);
  }
  HIDE_SPRITES;

  transition_wipe_up();
  CLEAR_BKG;

  draw_text(0, 0, player_bank->name);
  draw_money(player_bank->money, 0, 1);
  draw_text(2, 3, "* SLOT MACHINE *");
  draw_text(2, 5, "PLACE YOUR BET");
  draw_text(1, 6, "UP/DN:SELECT  A:GO");

  while (bet_options[selected] > player_bank->money && selected > 0)
    selected--;

  while (1) {
    for (i = 0; i < BET_OPTIONS_COUNT; i++) {
      uint8_t row = 9 + i;
      if (bet_options[i] > player_bank->money) {
        draw_text(3, row, "-- (NO FUNDS)  ");
      } else {
        draw_text(2, row, (i == selected) ? "> BET:" : "  BET:");
        draw_money(bet_options[i], 9, row);
      }
    }

    draw_text(2, 15, "WIN = BET x 10");

    prev_keys = keys;
    keys = joypad();
    keys_pressed = (keys ^ prev_keys) & keys;

    if (keys_pressed & J_SELECT)
      return 0;
    if (keys_pressed & J_UP) {
      int8_t next = (int8_t)selected - 1;
      while (next >= 0 && bet_options[next] > player_bank->money)
        next--;
      if (next >= 0) {
        selected = (uint8_t)next;
        play_select();
      }
    }
    if (keys_pressed & J_DOWN) {
      uint8_t next = selected + 1;
      while (next < BET_OPTIONS_COUNT && bet_options[next] > player_bank->money)
        next++;
      if (next < BET_OPTIONS_COUNT) {
        selected = next;
        play_select();
      }
    }
    if (keys_pressed & J_A && bet_options[selected] <= player_bank->money) {
      play_confirm();
      return bet_options[selected];
    }
    wait_vbl_done();
  }
}

#include "../assets/border.h"

static void init_slot(void) {
  uint8_t i;
  slot_t *ptr;

  CLEAR_BKG;
  audio_init();

  set_bkg_data(SLOT_BRD_TL, 1, win_brd_tl);
  set_bkg_data(SLOT_BRD_TR, 1, win_brd_tr);
  set_bkg_data(SLOT_BRD_BL, 1, win_brd_bl);
  set_bkg_data(SLOT_BRD_BR, 1, win_brd_br);
  set_bkg_data(SLOT_BRD_H, 1, win_brd_h);
  set_bkg_data(SLOT_BRD_HB, 1, win_brd_hb);
  set_bkg_data(SLOT_BRD_VL, 1, win_brd_vl);
  set_bkg_data(SLOT_BRD_VR, 1, win_brd_vr);
  set_bkg_data(SLOT_BRD_DI, slotBorder_TILE_COUNT, slotBorder_tiles);
  set_bkg_data(SLOT_BUTTON, button_TILE_COUNT, button_tiles);
  set_bkg_data(SLOT_BORDER, border_TILE_COUNT, border_tiles);

  set_sprite_data((uint8_t)(SLOT_SPR_BASE + 0), 1, star_tile);
  set_sprite_data((uint8_t)(SLOT_SPR_BASE + 1), 1, cross_tile);
  set_sprite_data((uint8_t)(SLOT_SPR_BASE + 2), 1, circle_tile);
  set_sprite_data((uint8_t)(SLOT_SPR_BASE + 3), 1, epitech_tile);

  ptr = slots_array;
  for (i = 0; i < 16; i++) {
    set_sprite_tile(i, SLOT_SPR_BASE);
    ptr->x = slot_pos[i >> 2];
    ptr->y = 80 + ((i & 3) << 4);
    ptr->type = (uint8_t)rand() & 3;
    set_sprite_tile(i, SLOT_SPR_BASE + ptr->type);
    if (ptr->y < TARGET_Y - 2 || ptr->y > TARGET_Y + 4)
      move_sprite(i, 0, 0);
    else
      move_sprite(i, ptr->x, ptr->y);
    ptr++;
  }

  draw_text(4, 5, "SLOT MACHINE");
  set_bkg_tile_xy(4, 12, SLOT_BRD_DI);
  set_bkg_tile_xy(14, 12, SLOT_BRD_DI);

  for (int i = 4; i < 14; i++)
    set_bkg_tile_xy(i, 13, SLOT_BORDER);

  for (int i = 13; i < 18; i++)
    set_bkg_tile_xy(3, i, SLOT_BRD_VR);
  for (int i = 14; i < 18; i++)
    set_bkg_tile_xy(13, i, SLOT_BRD_VR);
  for (int i = 13; i < 18; i++)
    set_bkg_tile_xy(14, i, SLOT_BRD_VR);
  for (int i = 8; i < 18; i++)
    set_bkg_tile_xy(15, i, SLOT_BRD_VR);
  for (int i = 6; i < 15; i++)
    set_bkg_tile_xy(i, 8, SLOT_BRD_H);

  set_bkg_tile_xy(5, 8, SLOT_BRD_DI);
  set_bkg_tile_xy(15, 8, SLOT_BRD_DI);
  set_bkg_tile_xy(6, 12, SLOT_BUTTON);
  set_bkg_tile_xy(8, 12, SLOT_BUTTON);
  set_bkg_tile_xy(12, 12, SLOT_BUTTON);

  set_bkg_tiles(5, 9, 10, 3, slot_machine_border);
  SHOW_SPRITES;
}

static uint8_t check_win(void) {
  uint8_t winning_types[4];
  uint8_t found = 0;
  uint8_t i;

  for (i = 0; i < 16; i++) {
    if (slots_array[i].y == TARGET_Y) {
      winning_types[found] = slots_array[i].type;
      found++;
    }
  }
  if (found != 4)
    return LOOSE;
  if (winning_types[0] == winning_types[1] &&
      winning_types[1] == winning_types[2] &&
      winning_types[2] == winning_types[3])
    return WIN;
  return LOOSE;
}

static uint8_t check_near_win(void) {
  uint8_t types[3];
  uint8_t found = 0;
  uint8_t i;
  for (i = 0; i < 12; i++) {
    if (slots_array[i].y == TARGET_Y) {
      types[found] = slots_array[i].type;
      found++;
    }
  }
  if (found == 3 && types[0] == types[1] && types[1] == types[2])
    return 1;
  return 0;
}

static void stop_slot(uint8_t col_start) {
  uint8_t i;
  slot_t *col = &slots_array[col_start];
  uint8_t best_idx = 0;
  uint8_t min_dist = 255;

  for (i = 0; i < 4; i++) {
    uint8_t dist =
        (col[i].y > TARGET_Y) ? (col[i].y - TARGET_Y) : (TARGET_Y - col[i].y);
    if (dist < min_dist) {
      min_dist = dist;
      best_idx = i;
    }
  }

  while (col[best_idx].y != TARGET_Y) {
    int8_t step = (col[best_idx].y < TARGET_Y) ? 1 : -1;
    for (i = 0; i < 4; i++) {
      col[i].y += step;
      if (col[i].y >= 144)
        col[i].y = 80;
      else if (col[i].y < 80)
        col[i].y = 143;
      if (col[i].y < TARGET_Y - 2 || col[i].y > TARGET_Y + 4)
        move_sprite(col_start + i, 0, 0);
      else
        move_sprite(col_start + i, col[i].x, col[i].y);
    }
    wait_vbl_done();
  }

  play_stop_clack();

  for (i = 0; i < 4; i++) {
    if (i != best_idx) {
      col[i].y = 0;
      move_sprite(col_start + i, 0, 0);
    }
  }
}

uint8_t slot_machine(bank_t *player_bank) {
  uint8_t keys = 0, prev_keys = 0, keys_pressed = 0;
  uint8_t stop_col = 0;
  uint8_t cooldown = 0;
  uint8_t target_type = 255;
  uint8_t spin_tick = 0;
  uint8_t bet;

  bet = choose_bet_slot(player_bank);
  if (bet == 0)
    return MENU;

  player_bank->money -= bet;

  init_slot();
  draw_text(0, 0, player_bank->name);
  draw_money(player_bank->money, 0, 1);
  draw_text(0, 2, "BET:");
  draw_money(bet, 5, 2);
  draw_text(0, 3, "B:STOP COL  SEL:QUIT");

  while (!(keys & J_SELECT)) {
    slot_t *ptr;
    uint8_t i;

    prev_keys = keys;
    keys = joypad();
    keys_pressed = (keys ^ prev_keys) & keys;

    if (cooldown > 0)
      cooldown--;
    spin_tick++;
    if (spin_tick >= 12 && stop_col < 16) {
      play_spin_tick();
      spin_tick = 0;
    }

    if ((keys_pressed & J_B) && (cooldown == 0)) {
      if (stop_col < 16) {
        stop_slot(stop_col);
        if (stop_col == 0) {
          for (i = 0; i < 4; i++) {
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
        uint8_t near = check_near_win();
        uint8_t won = check_win();

        delay(400);

        if (won) {
          uint8_t winnings = bet * 10;
          transition_flash(5);
          player_bank->money += winnings;
          draw_money(player_bank->money, 0, 1);
          draw_text(5, 14, "JACKPOT!");
          draw_text(8, 15, "x10");
          win_animation();
        } else {
          if (near) {
            transition_shake();
            play_near_miss();
            draw_text(5, 14, "SO CLOSE");
            draw_text(8, 15, "...!");
            delay(600);
          } else {
            play_loose();
            draw_text(5, 14, "NO LUCK");
            draw_text(8, 15, "...");
          }
          delay(800);
        }
        if (player_bank->money == 0) {
          transition_wipe_down();
          play_game_over();
          draw_text(2, 8, "GAME OVER!");
          draw_text(1, 10, "NO MORE FUNDS");
          delay(2000);
          return MENU;
        }
        return SLOT;
      }
    }
    ptr = slots_array + stop_col;
    for (i = stop_col; i < 16; i++) {
      ptr->y += SPIN_TICK_RATE;
      if (ptr->y >= 144) {
        if (target_type != 255 && ((uint8_t)rand() & 1) == 0) {
          ptr->type = target_type;
        } else {
          ptr->type = (uint8_t)rand() & 3;
        }
        set_sprite_tile(i, SLOT_SPR_BASE + ptr->type);
        ptr->y = 80;
      }
      if (ptr->y < TARGET_Y - 2 || ptr->y > TARGET_Y + 4)
        move_sprite(i, 0, 0);
      else
        move_sprite(i, ptr->x, ptr->y);
      ptr++;
    }

    wait_vbl_done();
  }
  player_bank->money += bet;
  return MENU;
}
