#include <gb/gb.h>

#include "../casino.h"
#include "../save/save.h"
#include "../utils/utils.h"
#include "games.h"
#include "menu.h"

uint8_t leaderboard(bank_t *player_bank) {
  save_slot_t slots[MAX_SAVES];
  uint8_t order[MAX_SAVES];
  uint8_t valid[MAX_SAVES];
  uint8_t n, i, j, tmp;
  uint8_t keys = 0;

  (void)player_bank;

  n = 0;
  for (i = 0; i < MAX_SAVES; i++) {
    order[i] = i;
    valid[i] = save_read_slot(i, &slots[i]);
    if (valid[i])
      n++;
  }

  for (i = 0; i < MAX_SAVES - 1; i++) {
    for (j = 0; j < MAX_SAVES - 1 - i; j++) {
      uint8_t a = order[j], b = order[j + 1];
      uint8_t va = valid[a], vb = valid[b];
      uint8_t swap = 0;
      if (!va && vb)
        swap = 1;
      else if (va && vb && slots[a].money < slots[b].money)
        swap = 1;
      if (swap) {
        tmp = order[j];
        order[j] = order[j + 1];
        order[j + 1] = tmp;
      }
    }
  }

  CLEAR_BKG;
  draw_text(2, 0, "* HIGH SCORES *");
  draw_text(0, 1, "--------------------");

  if (!n) {
    draw_text(3, 8, "NO SAVES YET");
  } else {
    for (i = 0; i < n && i < 8; i++) {
      uint8_t s = order[i];
      uint8_t row = 3 + i;
      set_bkg_tile_xy(0, row, ('1' + i) - 32);
      set_bkg_tile_xy(1, row, '.' - 32);
      draw_text(3, row, slots[s].name);
      draw_money(slots[s].money, 10, row);
    }
  }

  draw_text(0, 16, "--------------------");
  draw_text(1, 17, "ANY KEY TO RETURN");

  wait_pad_release();
  while (!joypad())
    wait_vbl_done();
  wait_pad_release();
  return MENU;
}
