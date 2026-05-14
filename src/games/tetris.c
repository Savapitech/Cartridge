#include <gb/gb.h>
#include <rand.h>
#include <stdint.h>
#include <string.h>

#include "../audio/audio.h"
#include "../casino.h"
#include "../utils/transitions.h"
#include "../utils/utils.h"
#include "menu.h"
#include "tetris.h"

static const uint8_t block_tile[16] = {
    0xFF, 0xFF, /* row 0: solid border */
    0xFF, 0x81, /* row 1: left+right in color3, interior color1 */
    0xFF, 0x81, 0xFF, 0x81, 0xFF, 0x81,
    0xFF, 0x81, 0xFF, 0x81, 0xFF, 0xFF, /* row 7: solid border */
};

static const uint16_t PIECES[T_PIECES][4] = {
    {0x0F00, 0x2222, 0x00F0, 0x4444}, // I
    {0x6600, 0x6600, 0x6600, 0x6600}, // O
    {0x0E40, 0x4C40, 0x4E00, 0x4640}, // T
    {0x06C0, 0x8C40, 0x06C0, 0x8C40}, // S
    {0x0C60, 0x4C80, 0x0C60, 0x4C80}, // Z
    {0x08E0, 0xC880, 0x0E20, 0x44C0}, // J
    {0x02E0, 0x88C0, 0x0E80, 0x6440}  // L
};

/* frames per drop, by level 0- to19 */
static const uint8_t GRAVITY[20] = {
    48, 43, 38, 33, 28, 23, 18, 13, 8, 6, 5, 5, 5, 4, 4, 4, 3, 3, 3, 2,
};

static const uint16_t LINE_PTS[5] = {0, 40, 100, 300, 1200};

static uint8_t board[BOARD_H][BOARD_W];
static uint8_t cur_type, cur_rot;
static int8_t cur_x, cur_y;
static uint8_t next_type;
static uint32_t score;
static uint8_t level;
static uint16_t total_lines;
static uint8_t grav_timer;

static uint8_t row_bits(uint8_t type, uint8_t rot, uint8_t r) {
  return (uint8_t)(PIECES[type][rot] >> (12 - (uint8_t)(r << 2))) & 0x0F;
}

static uint8_t fits(uint8_t type, uint8_t rot, int8_t px, int8_t py) {
  uint8_t r, c;
  for (r = 0; r < 4; r++) {
    uint8_t row = row_bits(type, rot, r);
    if (!row)
      continue;
    for (c = 0; c < 4; c++) {
      if (!(row & (8u >> c)))
        continue;
      int8_t bx = px + (int8_t)c;
      int8_t by = py + (int8_t)r;
      if (bx < 0 || bx >= (int8_t)BOARD_W)
        return 0;
      if (by >= (int8_t)BOARD_H)
        return 0;
      if (by >= 0 && board[(uint8_t)by][(uint8_t)bx])
        return 0;
    }
  }
  return 1;
}

static void draw_cell(uint8_t bx, uint8_t by, uint8_t on) {
  set_bkg_tile_xy(bx + SCR_BOARD_X, by + SCR_BOARD_Y, on ? T_BLOCK : ' ' - 32);
}

static void draw_piece(uint8_t type, uint8_t rot, int8_t px, int8_t py,
                       uint8_t on) {
  uint8_t r, c;
  for (r = 0; r < 4; r++) {
    uint8_t row = row_bits(type, rot, r);
    if (!row)
      continue;
    for (c = 0; c < 4; c++) {
      if (!(row & (8u >> c)))
        continue;
      int8_t bx = px + (int8_t)c;
      int8_t by = py + (int8_t)r;
      if (bx >= 0 && bx < (int8_t)BOARD_W && by >= 0 && by < (int8_t)BOARD_H)
        draw_cell((uint8_t)bx, (uint8_t)by, on);
    }
  }
}

static void draw_board_full(void) {
  uint8_t r, c;
  for (r = 0; r < BOARD_H; r++)
    for (c = 0; c < BOARD_W; c++)
      draw_cell(c, r, board[r][c] ? 1 : 0);
}

static void draw_next(void) {
  uint8_t r, c;
  for (r = 0; r < 4; r++)
    for (c = 0; c < 4; c++) {
      uint8_t on = (row_bits(next_type, 0, r) >> (3 - c)) & 1;
      set_bkg_tile_xy(14 + c, 9 + r, on ? T_BLOCK : ' ' - 32);
    }
}

static void draw_score_ui(void) {
  draw_text(12, 0, "SC:");
  draw_money(score, 12, 1);
  draw_text(12, 3, "LVL:");
  set_bkg_tile_xy(16, 3, (level >= 10 ? '0' + level / 10 : ' ') - 32);
  set_bkg_tile_xy(17, 3, '0' + level % 10 - 32);
  draw_text(12, 5, "LINES:");
  draw_money(total_lines, 12, 6);
  draw_text(12, 8, "NEXT:");
}

static void draw_border(void) {
  uint8_t i;
  for (i = 0; i < BOARD_H + 2; i++) {
    set_bkg_tile_xy(0, i, '|' - 32);
    set_bkg_tile_xy(11, i, '|' - 32);
  }
  for (i = 0; i <= 11; i++)
    set_bkg_tile_xy(i, BOARD_H + 1, '=' - 32);
}

static void lock_piece(void) {
  uint8_t r, c;
  for (r = 0; r < 4; r++) {
    uint8_t row = row_bits(cur_type, cur_rot, r);
    if (!row)
      continue;
    for (c = 0; c < 4; c++) {
      if (!(row & (8u >> c)))
        continue;
      int8_t bx = cur_x + (int8_t)c;
      int8_t by = cur_y + (int8_t)r;
      if (bx >= 0 && bx < (int8_t)BOARD_W && by >= 0 && by < (int8_t)BOARD_H)
        board[(uint8_t)by][(uint8_t)bx] = cur_type + 1;
    }
  }
}

static uint8_t clear_lines(void) {
  uint8_t cleared = 0;
  int8_t r;
  for (r = (int8_t)(BOARD_H - 1); r >= 0; r--) {
    uint8_t full = 1, c;
    for (c = 0; c < BOARD_W; c++)
      if (!board[(uint8_t)r][c]) {
        full = 0;
        break;
      }
    if (full) {
      int8_t s;
      for (s = r; s > 0; s--)
        memcpy(board[(uint8_t)s], board[(uint8_t)(s - 1)], BOARD_W);
      memset(board[0], 0, BOARD_W);
      r++;
      cleared++;
    }
  }
  return cleared;
}

static void update_score(uint8_t lines) {
  if (!lines)
    return;
  score += (uint32_t)LINE_PTS[lines] * ((uint32_t)level + 1);
  total_lines += lines;
  level = (uint8_t)(total_lines / 10);
  if (level > 19)
    level = 19;
}

static uint8_t spawn_piece(void) {
  cur_type = next_type;
  cur_rot = 0;
  cur_x = (int8_t)((BOARD_W - 4) / 2);
  cur_y = 0;
  next_type = (uint8_t)rand() % T_PIECES;
  grav_timer = GRAVITY[level];
  draw_next();
  return fits(cur_type, cur_rot, cur_x, cur_y);
}

static void play_lock(void) { ch2_play(NOTE_C4_LO, NOTE_C4_HI, 2, 4, 8, 0, 4); }

static void play_clear(uint8_t n) {
  if (n == 4) {
    transition_flash(3);
    play_win();
  } else {
    uint8_t i;
    for (i = 0; i < n; i++) {
      ch1_play(NOTE_C4_LO, NOTE_C4_HI + i, 2, 8, 12, 0, 3);
      delay(80);
    }
    ch1_play(NOTE_G4_LO, NOTE_G4_HI, 2, 12, 14, 0, 2);
  }
}

uint8_t tetris(bank_t *player_bank) {
  uint8_t keys = 0, prev_keys = 0, pressed;
  uint8_t game_over = 0;
  uint8_t das_timer = 0;
  uint8_t das_dir = 0;
  uint32_t award;

  audio_init();
  set_bkg_data(T_BLOCK, 1, block_tile);

  memset(board, 0, sizeof(board));
  score = 0;
  level = 0;
  total_lines = 0;
  next_type = (uint8_t)rand() % T_PIECES;

  CLEAR_BKG;
  HIDE_SPRITES;
  draw_border();
  draw_score_ui();

  if (!spawn_piece())
    return MENU;
  draw_board_full();
  draw_piece(cur_type, cur_rot, cur_x, cur_y, 1);

  while (!game_over) {
    wait_vbl_done();

    prev_keys = keys;
    keys = joypad();
    pressed = (keys ^ prev_keys) & keys;

    if (pressed & J_SELECT)
      break;

    if (pressed & J_A) {
      uint8_t nr = (cur_rot + 1) & 3;
      if (fits(cur_type, nr, cur_x, cur_y)) {
        draw_piece(cur_type, cur_rot, cur_x, cur_y, 0);
        cur_rot = nr;
        draw_piece(cur_type, cur_rot, cur_x, cur_y, 1);
        play_select();
      }
    }
    if (pressed & J_B) {
      uint8_t nr = (cur_rot + 3) & 3;
      if (fits(cur_type, nr, cur_x, cur_y)) {
        draw_piece(cur_type, cur_rot, cur_x, cur_y, 0);
        cur_rot = nr;
        draw_piece(cur_type, cur_rot, cur_x, cur_y, 1);
        play_select();
      }
    }

    if (pressed & J_LEFT) {
      das_dir = 1;
      das_timer = DAS_INIT;
      if (fits(cur_type, cur_rot, cur_x - 1, cur_y)) {
        draw_piece(cur_type, cur_rot, cur_x, cur_y, 0);
        cur_x--;
        draw_piece(cur_type, cur_rot, cur_x, cur_y, 1);
      }
    } else if (pressed & J_RIGHT) {
      das_dir = 2;
      das_timer = DAS_INIT;
      if (fits(cur_type, cur_rot, cur_x + 1, cur_y)) {
        draw_piece(cur_type, cur_rot, cur_x, cur_y, 0);
        cur_x++;
        draw_piece(cur_type, cur_rot, cur_x, cur_y, 1);
      }
    } else if (!(keys & J_LEFT) && !(keys & J_RIGHT)) {
      das_dir = 0;
    }

    if (das_dir && das_timer > 0) {
      das_timer--;
      if (das_timer == 0) {
        int8_t dx = (das_dir == 1) ? -1 : 1;
        das_timer = DAS_REP;
        if (fits(cur_type, cur_rot, cur_x + dx, cur_y)) {
          draw_piece(cur_type, cur_rot, cur_x, cur_y, 0);
          cur_x += dx;
          draw_piece(cur_type, cur_rot, cur_x, cur_y, 1);
        }
      }
    }

    if ((keys & J_DOWN) && fits(cur_type, cur_rot, cur_x, cur_y + 1)) {
      draw_piece(cur_type, cur_rot, cur_x, cur_y, 0);
      cur_y++;
      draw_piece(cur_type, cur_rot, cur_x, cur_y, 1);
      grav_timer = GRAVITY[level];
      score++;
    }

    if (grav_timer > 0)
      grav_timer--;
    if (grav_timer == 0) {
      if (fits(cur_type, cur_rot, cur_x, cur_y + 1)) {
        draw_piece(cur_type, cur_rot, cur_x, cur_y, 0);
        cur_y++;
        draw_piece(cur_type, cur_rot, cur_x, cur_y, 1);
        grav_timer = GRAVITY[level];
      } else {
        lock_piece();
        play_lock();
        {
          uint8_t cleared = clear_lines();
          if (cleared) {
            play_clear(cleared);
            update_score(cleared);
            draw_board_full();
            draw_score_ui();
          }
        }
        if (!spawn_piece()) {
          game_over = 1;
        } else {
          draw_piece(cur_type, cur_rot, cur_x, cur_y, 1);
        }
      }
    }
  }

  play_game_over();
  CLEAR_BKG;
  draw_text(4, 5, "GAME OVER!");
  draw_text(1, 6, "SCORE:");
  draw_money(score, 8, 6);
  award = score / 10;
  if (award > 0) {
    player_bank->money += award;
    draw_text(1, 8, "EARNED:");
    draw_money(award, 8, 8);
  }
  draw_text(1, 10, "ANY KEY TO RETURN");

  wait_pad_release();
  while (!joypad())
    wait_vbl_done();
  wait_pad_release();

  return MENU;
}
