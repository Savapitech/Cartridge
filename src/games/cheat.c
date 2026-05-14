#include <gb/gb.h>
#include <string.h>

#include "../casino.h"
#include "../save/save.h"
#include "../utils/get_input.h"
#include "../utils/utils.h"
#include "games.h"
#include "menu.h"
#include "tetris.h"

uint8_t cheat_code(bank_t *player_bank) {
  char buffer[INPUT_BUFFER] = INPUT_HANDLER;
  uint8_t len, i;

  CLEAR_BKG;
  draw_text(0, 0, player_bank->name);
  draw_money(player_bank->money, 0, 1);
  draw_text(3, 3, "PLAYER SELECT");
  draw_text(2, 4, "ENTER NAME:");
  draw_text(0, 14, "B:LETTER  A:CONFIRM");
  draw_text(1, 15, "UP/DN/LT/RT:MOVE");

  input_handler(buffer);

  for (i = 0; i < 10 && buffer[i] != '_'; i++)
    ;
  buffer[i] = '\0';
  len = i;

  if (len == 0)
    return MENU;

  if (strncmp(buffer, "tetris", 6) == 0)
    return tetris(player_bank);

  if (strncmp(buffer, "epitech", 7) == 0) {
    player_bank->money += 42;
    return MENU;
  }

  save_write(player_bank);

  if (!save_load(buffer, player_bank)) {
    strncpy(player_bank->name, buffer, 10);
    player_bank->name[10] = '\0';
    player_bank->name_size = len;
    player_bank->money = 0;
    save_write(player_bank);
  }

  return MENU;
}