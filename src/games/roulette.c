#include <gb/gb.h>
#include <rand.h>
#include <stdint.h>

#include "../assets/roulette_asset.h"
#include "../audio/audio.h"
#include "../casino.h"
#include "../utils/transitions.h"
#include "../utils/utils.h"
#include "games.h"
#include "menu.h"
#include "roulette.h"

static const uint8_t orb_x[16] = {66, 64, 60, 54, 46, 38, 32, 28,
                                  26, 28, 32, 38, 46, 54, 60, 64};
static const uint8_t orb_y[16] = {54, 62, 68, 72, 74, 72, 68, 62,
                                  54, 46, 40, 36, 34, 36, 40, 46};

static const char *bet_labels[BET_TYPES] = {"RED    ", "BLACK  ", "GREEN  ",
                                            "ODD    ", "EVEN   ", "LOW    ",
                                            "HIGH   "};

static const char *pay_strs[BET_TYPES] = {"PAYS:  x2", "PAYS:  x2", "PAYS: x35",
                                          "PAYS:  x2", "PAYS:  x2", "PAYS:  x2",
                                          "PAYS:  x2"};

static const char *col_strs[3] = {"GREEN", "RED  ", "BLACK"};

typedef struct {
  uint8_t lo, hi, dur;
} note_t;

#define N(x, d)                                                                \
  { (uint8_t)((x) & 0xFF), (uint8_t)(((x) >> 8) | 0x80), (d) }
#define REST(d)                                                                \
  { 0, 0, (d) }

#define MELODY_LEN 13
static const note_t melody[MELODY_LEN] = {
    N(1797, 12), N(1849, 12), N(1881, 18), REST(6),     N(1849, 12),
    N(1825, 12), N(1797, 24), N(1714, 12), N(1783, 12), N(1825, 12),
    N(1783, 12), N(1714, 24), REST(18),
};

static uint8_t mus_step = 0;
static uint8_t mus_timer = 0;

static void music_init(void) {
  audio_init();
  mus_step = 0;
  mus_timer = 0;
}

static void music_tick(void) {
  const note_t *n;
  if (mus_timer > 0) {
    mus_timer--;
    return;
  }
  n = &melody[mus_step];
  if (n->hi) {
    NR21_REG = 0x80;
    NR22_REG = 0x62;
    NR23_REG = n->lo;
    NR24_REG = n->hi;
  } else {
    NR22_REG = 0x00;
  }
  mus_timer = n->dur - 1;
  if (++mus_step >= MELODY_LEN)
    mus_step = 0;
}

static void snd_tick(void) {
  NR41_REG = 0x00;
  NR42_REG = 0x51;
  NR43_REG = 0x77;
  NR44_REG = 0x80;
}

static void load_tiles(void) {
  set_bkg_data(T_WHEEL_RED, 1, wheel_tile_red);
  set_bkg_data(T_WHEEL_BLACK, 1, wheel_tile_black);
  set_bkg_data(T_WHEEL_GREEN, 1, wheel_tile_green);
  set_bkg_data(T_FELT, 1, felt_tile);
  set_bkg_data(T_BRD_TL, 1, brd_tl);
  set_bkg_data(T_BRD_TR, 1, brd_tr);
  set_bkg_data(T_BRD_BL, 1, brd_bl);
  set_bkg_data(T_BRD_BR, 1, brd_br);
  set_bkg_data(T_BRD_H, 1, brd_h);
  set_bkg_data(T_BRD_H_B, 1, brd_h_b);
  set_bkg_data(T_BRD_VL, 1, brd_vl);
  set_bkg_data(T_BRD_VR, 1, brd_vr);
  set_bkg_data(T_DIAMOND, 1, diamond_tile);
  set_bkg_data(T_CHIP, 1, chip_tile);
  set_sprite_data(BALL_TILE_IDX, 1, ball_tile);
  set_sprite_tile(SPR_BALL, BALL_TILE_IDX);
}

static void draw_bg(void) {
  uint8_t x, y;

  for (y = 0; y < 18; y++)
    for (x = 0; x < 20; x++)
      set_bkg_tile_xy(x, y, T_FELT);

  set_bkg_tile_xy(0, 0, T_BRD_TL);
  set_bkg_tile_xy(19, 0, T_BRD_TR);
  set_bkg_tile_xy(0, 17, T_BRD_BL);
  set_bkg_tile_xy(19, 17, T_BRD_BR);

  for (x = 1; x < 19; x++) {
    set_bkg_tile_xy(x, 0, T_BRD_H);
    set_bkg_tile_xy(x, 17, T_BRD_H_B);
  }
  for (y = 1; y < 17; y++) {
    set_bkg_tile_xy(0, y, T_BRD_VL);
    set_bkg_tile_xy(19, y, T_BRD_VR);
  }

  for (y = 1; y < 17; y++)
    set_bkg_tile_xy(8, y, T_BRD_VL);
  set_bkg_tile_xy(8, 0, T_BRD_H);
  set_bkg_tile_xy(8, 17, T_BRD_H_B);

  set_bkg_tile_xy(1, 14, T_DIAMOND);
  set_bkg_tile_xy(6, 14, T_DIAMOND);
  set_bkg_tile_xy(1, 16, T_DIAMOND);
  set_bkg_tile_xy(6, 16, T_DIAMOND);
  set_bkg_tile_xy(18, 15, T_CHIP);
  set_bkg_tile_xy(18, 16, T_CHIP);
}

static void draw_wheel(void) {
  static const uint8_t wmap[6][6] = {
      {T_WHEEL_GREEN, T_WHEEL_RED, T_WHEEL_BLACK, T_WHEEL_RED, T_WHEEL_BLACK,
       T_WHEEL_GREEN},
      {T_WHEEL_BLACK, T_FELT, T_FELT, T_FELT, T_FELT, T_WHEEL_RED},
      {T_WHEEL_RED, T_FELT, T_WHEEL_BLACK, T_WHEEL_BLACK, T_FELT,
       T_WHEEL_BLACK},
      {T_WHEEL_BLACK, T_FELT, T_WHEEL_BLACK, T_WHEEL_BLACK, T_FELT,
       T_WHEEL_RED},
      {T_WHEEL_RED, T_FELT, T_FELT, T_FELT, T_FELT, T_WHEEL_BLACK},
      {T_WHEEL_GREEN, T_WHEEL_BLACK, T_WHEEL_RED, T_WHEEL_BLACK, T_WHEEL_RED,
       T_WHEEL_GREEN},
  };
  uint8_t r;
  for (r = 0; r < 6; r++)
    set_bkg_tiles(WHEEL_TX, WHEEL_TY + r, 6, 1, wmap[r]);
}

static void draw_static_ui(void) {
  draw_text(UI_X, ROW_TITLE - 1, "----------");
  draw_text(UI_X, ROW_TITLE, " ROULETTE ");
  draw_text(UI_X, ROW_SEP1, "----------");
  draw_text(UI_X, ROW_SEP2, "----------");
  draw_text(UI_X, ROW_HINT1, "<> :TYPE  ");
  draw_text(UI_X, ROW_HINT2, "^v :AMT   ");
  draw_text(UI_X, ROW_HINT3, "B  :SPIN  ");
  draw_text(UI_X, ROW_HINT3 + 1, "SEL:MENU  ");
}

static void draw_bet_ui(uint8_t sel, uint32_t bet) {
  draw_text(UI_X, ROW_BET, "<");
  draw_text(UI_X + 1, ROW_BET, bet_labels[sel]);
  draw_text(UI_X + 8, ROW_BET, " >");
  draw_text(UI_X, ROW_PAY, pay_strs[sel]);
  draw_text(UI_X, ROW_AMT, "AMT:      ");
  draw_money(bet, UI_X + 4, ROW_AMT);
}

static void draw_header_money(uint32_t money) {
  draw_text(10, 0, "$");
  draw_money(money, 11, 0);
}

static void clear_result(void) {
  fill_bkg_rect(1, ROW_RESULT, 18, 1, ' ' - 32);
  fill_bkg_rect(1, ROW_OUTCOME, 18, 1, ' ' - 32);
}

static void spin_ball(void) {
  uint8_t pos = 0;
  uint8_t step = 0;
  uint8_t total = SPIN_FAST_STEPS + SPIN_MED_STEPS + SPIN_SLOW_STEPS;
  uint8_t f, period;

  while (step < total) {
    period = (step < SPIN_FAST_STEPS)                    ? 1
             : (step < SPIN_FAST_STEPS + SPIN_MED_STEPS) ? 2
                                                         : 4;
    for (f = 0; f < period; f++) {
      wait_vbl_done();
      music_tick();
    }
    pos = (pos + 1) & 15;
    move_sprite(SPR_BALL, orb_x[pos], orb_y[pos]);
    if ((step & 7) == 0)
      snd_tick();
    step++;
  }
  move_sprite(SPR_BALL, 0, 0);
}

static uint32_t calc_payout(uint8_t bet_type, uint8_t result, uint32_t bet) {
  uint8_t col = roulette_colors[result];
  if (bet_type == BET_RED && col == COL_RED)
    return bet * PAY_COLOR;
  if (bet_type == BET_BLACK && col == COL_BLACK)
    return bet * PAY_COLOR;
  if (bet_type == BET_GREEN && col == COL_GREEN)
    return bet * PAY_GREEN;
  if (bet_type == BET_ODD && result && (result & 1))
    return bet * PAY_ODDEVEN;
  if (bet_type == BET_EVEN && result && !(result & 1))
    return bet * PAY_ODDEVEN;
  if (bet_type == BET_LOW && result >= 1 && result <= 18)
    return bet * PAY_LOWHIGH;
  if (bet_type == BET_HIGH && result >= 19)
    return bet * PAY_LOWHIGH;
  return 0;
}

uint8_t roulette(bank_t *player_bank) {
  uint8_t sel = BET_RED;
  uint32_t bet = BET_MIN;
  uint8_t keys = 0;
  uint8_t pkeys = 0;
  uint8_t pressed;

  transition_wipe_down();
  CLEAR_BKG;
  HIDE_WIN;
  HIDE_SPRITES;

  load_tiles();
  draw_bg();
  draw_wheel();
  draw_static_ui();
  music_init();

  draw_text(1, 0, player_bank->name);
  draw_header_money(player_bank->money);
  draw_bet_ui(sel, bet);
  move_sprite(SPR_BALL, 0, 0);
  OBP0_REG = 0xD2;
  SHOW_SPRITES;
  SHOW_BKG;

  while (1) {
    pkeys = keys;
    keys = joypad();
    pressed = (keys ^ pkeys) & keys;

    if (pressed & J_SELECT)
      return MENU;
    if (pressed & J_A)
      return ROULETTE;

    if (pressed & J_RIGHT) {
      sel = (sel + 1 < BET_TYPES) ? sel + 1 : 0;
      draw_bet_ui(sel, bet);
      wait_pad_release();
    }
    if (pressed & J_LEFT) {
      sel = (sel > 0) ? sel - 1 : BET_TYPES - 1;
      draw_bet_ui(sel, bet);
      wait_pad_release();
    }
    if (pressed & J_UP) {
      if (bet + BET_STEP <= BET_MAX)
        bet += BET_STEP;
      draw_bet_ui(sel, bet);
      wait_pad_release();
    }
    if (pressed & J_DOWN) {
      if (bet > BET_MIN)
        bet -= BET_STEP;
      draw_bet_ui(sel, bet);
      wait_pad_release();
    }

    if (pressed & J_B) {
      if (player_bank->money < bet) {
        clear_result();
        draw_text(1, ROW_RESULT, "NO FUNDS! ");
        play_lose();
        wait_pad_release();
        continue;
      }

      player_bank->money -= bet;
      draw_header_money(player_bank->money);
      wait_pad_release();

      clear_result();
      spin_ball();

      {
        uint8_t result = roulette_numbers[(uint8_t)(rand() % 37)];
        uint32_t payout = calc_payout(sel, result, bet);

        clear_result();
        draw_text(1, ROW_RESULT, "RESULT: ");
        draw_money(result, 9, ROW_RESULT);
        draw_text(13, ROW_RESULT, col_strs[roulette_colors[result]]);

        if (payout > 0) {
          player_bank->money += payout;
          draw_header_money(player_bank->money);
          draw_text(1, ROW_OUTCOME, "WIN!  +");
          draw_money(payout, 8, ROW_OUTCOME);
          transition_flash(2);
          play_win();
        } else {
          draw_text(1, ROW_OUTCOME, "LOSE...           ");
          play_lose();
        }
      }

      {
        uint8_t d;
        for (d = 0; d < 90; d++) {
          wait_vbl_done();
          music_tick();
        }
      }
    }

    wait_vbl_done();
    music_tick();
  }
}
