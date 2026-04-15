#include <gb/gb.h>
#include <stdint.h>

#include "../casino.h"

const uint32_t powers_of_10[10] = {
    1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1};

void draw_money(uint32_t money, uint8_t x, uint8_t y) {
  uint8_t i;
  uint8_t digit;
  uint8_t started = 0;

  uint8_t draw_x = x;

  if (money == 0) {
    set_win_tile_xy(draw_x, y, '0' - 32);
    draw_x++;
  } else {
    for (i = 0; i < 10; i++) {
      digit = 0;

      while (money >= powers_of_10[i]) {
        money -= powers_of_10[i];
        digit++;
      }
      if (digit > 0)
        started = 1;
      if (started == 1) {
        set_win_tile_xy(draw_x, y, digit + '0' - 32);
        draw_x++;
      }
    }
  }
  while (draw_x < x + 10) {
    set_win_tile_xy(draw_x, y, ' ' - 32);
    draw_x++;
  }
}

uint32_t last_money = 0xFFFFFFFF;

void render_money(bank_t *player_bank) {
  if (player_bank->money != last_money) {
    draw_money(player_bank->money, 0, 1);
    last_money = player_bank->money;
  }
}
