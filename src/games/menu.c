#include <gb/gb.h>

#include "../casino.h"
#include "../utils/utils.h"
#include "games.h"
#include "menu.h"

uint8_t menu(bank_t *player_bank) {
  CLEAR_WIN;
  HIDE_SPRITES;
  draw_text(0, 0, player_bank->name);
  draw_money(player_bank->money, 0, 1);

  uint8_t game_idx = 0;
  uint8_t last_idx = MIN_GAME_IDX;
  uint8_t keys = 0;

  while (!(keys & J_SELECT)) {
    keys = joypad();

    if (keys & J_A) {
      wait_pad_release();
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
      draw_text(0, MENU_UP_TXT, "                    ");
      draw_text(0, MENU_MDL_TXT, "                    ");
      draw_text(0, MENU_DOWN_TXT, "                    ");

      uint8_t up_idx = (game_idx == 0) ? (GAME_TAB_SIZE - 1) : (game_idx - 1);
      uint8_t down_idx = (game_idx == GAME_TAB_SIZE - 1) ? 0 : (game_idx + 1);

      draw_text(10 - (game_tab[up_idx].name_size >> 1), MENU_UP_TXT,
                game_tab[up_idx].name);
      draw_text(10 - (game_tab[game_idx].name_size >> 1), MENU_MDL_TXT,
                game_tab[game_idx].name);
      draw_text(10 - (game_tab[game_idx].name_size >> 1) - 3, MENU_MDL_TXT,
                ARROW);
      draw_text(10 - (game_tab[down_idx].name_size >> 1), MENU_DOWN_TXT,
                game_tab[down_idx].name);

      last_idx = game_idx;
      wait_pad_release();
    }
    wait_vbl_done();
  }
  return game_idx;
}

/*TMP*/

uint8_t roulette(bank_t *player_bank) {
  draw_text(5, 5, "B");
  return MENU;
}

uint8_t black_jack(bank_t *player_bank) {
  draw_text(5, 5, "C");
  return MENU;
}

uint8_t tax_money(bank_t *player_bank) {
  player_bank->money++;
  return 0;
}
