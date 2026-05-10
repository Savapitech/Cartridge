#include <gb/gb.h>
#include <rand.h>

#include "../audio/audio.h"
#include "../casino.h"
#include "../utils/transitions.h"
#include "../utils/utils.h"
#include "games.h"
#include "menu.h"

/*
 * Screen layout (20 cols x 18 rows):
 *
 *  Row  0: +==================+
 *  Row  1: |  ** CASINO GB ** |
 *  Row  2: +==================+
 *  Row  3: |                  |
 *  Row  4: | <name>   $<money>|
 *  Row  5: |                  |
 *  Row  6: +------------------+
 *  Row  7: |                  |
 *  Row  8: |    <prev game>   |
 *  Row  9: | -> <CURR GAME>   |
 *  Row 10: |    <next game>   |
 *  Row 11: |                  |
 *  Row 12: +------------------+
 *  Row 13: |                  |
 *  Row 14: |  A   : SELECT    |
 *  Row 15: |  SEL : QUIT      |
 *  Row 16: |                  |
 *  Row 17: +==================+
 */

static void draw_border(void) {
  uint8_t i;

  draw_text(0, 0, "+==================+");
  draw_text(0, 1, "|  ** CASINO GB ** |");
  draw_text(0, 2, "+==================+");
  draw_text(0, 6, "+------------------+");
  draw_text(0, 12, "+------------------+");
  draw_text(0, 17, "+==================+");

  for (i = 3; i < 6; i++) {
    set_bkg_tile_xy(0, i, '|' - 32);
    set_bkg_tile_xy(19, i, '|' - 32);
  }
  for (i = 7; i < 12; i++) {
    set_bkg_tile_xy(0, i, '|' - 32);
    set_bkg_tile_xy(19, i, '|' - 32);
  }
  for (i = 13; i < 17; i++) {
    set_bkg_tile_xy(0, i, '|' - 32);
    set_bkg_tile_xy(19, i, '|' - 32);
  }
}

static void draw_player_info(bank_t *player_bank) {
  draw_text(2, 4, player_bank->name);
  draw_text(9, 4, "$");
  draw_money(player_bank->money, 10, 4);
}

static void draw_hint(void) {
  draw_text(2, 14, "A   : SELECT    ");
  draw_text(2, 15, "SEL : QUIT      ");
}

static void draw_game_list(uint8_t game_idx) {
  uint8_t up_idx = (game_idx == 0) ? (GAME_TAB_SIZE - 1) : (game_idx - 1);
  uint8_t down_idx = (game_idx == GAME_TAB_SIZE - 1) ? 0 : (game_idx + 1);

  draw_text(1, MENU_UP_TXT, "                  ");
  draw_text(1, MENU_MDL_TXT, "                  ");
  draw_text(1, MENU_DOWN_TXT, "                  ");

  draw_text(10 - (game_tab[up_idx].name_size >> 1), MENU_UP_TXT,
            game_tab[up_idx].name);
  draw_text(10 - (game_tab[game_idx].name_size >> 1), MENU_MDL_TXT,
            game_tab[game_idx].name);
  draw_text(10 - (game_tab[game_idx].name_size >> 1) - 3, MENU_MDL_TXT, ARROW);
  draw_text(10 - (game_tab[down_idx].name_size >> 1), MENU_DOWN_TXT,
            game_tab[down_idx].name);
}

uint8_t menu(bank_t *player_bank) {
  uint8_t game_idx = 0;
  uint8_t last_idx = MIN_GAME_IDX;
  uint8_t keys = 0;

  transition_wipe_up();
  CLEAR_BKG;
  HIDE_SPRITES;

  draw_border();
  draw_player_info(player_bank);
  draw_hint();

  while (!(keys & J_SELECT)) {
    keys = joypad();

    if (keys & J_A) {
      play_confirm();
      wait_pad_release();
      initrand(DIV_REG);
      return game_idx;
    }

    if (keys & J_DOWN)
      game_idx++;
    if (keys & J_UP)
      game_idx--;
    if (game_idx == GAME_TAB_SIZE)
      game_idx = 0;
    if (game_idx == MIN_GAME_IDX)
      game_idx = GAME_TAB_SIZE - 1;

    if (game_idx != last_idx) {
      draw_game_list(game_idx);
      last_idx = game_idx;
      play_select();
      wait_pad_release();
    }
    wait_vbl_done();
  }
  return game_idx;
}

uint8_t tax_money(bank_t *player_bank) {
  player_bank->money += 10;
  return 0;
}
