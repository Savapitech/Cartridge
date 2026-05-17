#include <gb/gb.h>
#include <rand.h>

#include "../assets/menu_banner.h"
#include "../audio/audio.h"
#include "../casino.h"
#include "../utils/transitions.h"
#include "../utils/utils.h"
#include "games.h"
#include "menu.h"

static void draw_banner(void) { draw_text(3, 1, "** CASINO GB **"); }

static void draw_border(void) {
  uint8_t i;

  draw_banner();

  draw_text(0, 5, "+------------------+");
  draw_text(0, 11, "+------------------+");
  draw_text(0, 17, "+==================+");

  for (i = 6; i < 11; i++) {
    set_bkg_tile_xy(0, i, '|' - 32);
    set_bkg_tile_xy(19, i, '|' - 32);
  }

  for (i = 12; i < 17; i++) {
    set_bkg_tile_xy(0, i, '|' - 32);
    set_bkg_tile_xy(19, i, '|' - 32);
  }
}

static void draw_player_info(bank_t *player_bank) {
  draw_text(2, 4, player_bank->name);
  draw_text(10, 4, "$");
  draw_money(player_bank->money, 11, 4);
}

static void draw_hint(void) {
  draw_text(2, 13, "A   : SELECT    ");
  draw_text(2, 14, "SEL : QUIT      ");
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

static void clear_banner_sprites(void) {
  uint8_t i;
  for (i = 0; i < 22; i++)
    move_sprite(i, 0, 0);
}

uint8_t menu(bank_t *player_bank) {
  uint8_t game_idx = 0;
  uint8_t last_idx = MIN_GAME_IDX;
  uint8_t keys = 0;

  CLEAR_BKG;
  SHOW_SPRITES;

  draw_border();
  draw_player_info(player_bank);
  draw_hint();

  load_banner_sprites();
  init_banner_lights();

  while (!(keys & J_SELECT)) {
    keys = joypad();

    if (keys & J_A) {
      play_confirm();
      wait_pad_release();
      initrand(DIV_REG);
      clear_banner_sprites();
      HIDE_SPRITES;
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

    update_banner_lights();
    wait_vbl_done();
  }
  clear_banner_sprites();
  HIDE_SPRITES;
  return game_idx;
}

uint8_t tax_money(bank_t *player_bank) {
  player_bank->money += 10;
  return 0;
}
