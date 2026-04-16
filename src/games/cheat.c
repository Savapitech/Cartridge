#include "../casino.h"
#include "../utils/get_input.h"
#include "../utils/utils.h"

#include <gb/gb.h>

#include <string.h>

uint8_t cheat_code(bank_t *player_bank) {
  char buffer[INPUT_BUFFER] = INPUT_HANDLER;
  CLEAR_WIN;
  draw_text(0, 0, player_bank->name);
  draw_money(player_bank->money, 0, 1);
  input_handler(buffer);

  if (strncmp(buffer, "epitech", 7) == 0)
    player_bank->money += 42;
  else if (player_bank->money >= 42)
    player_bank->money -= 42;
  else
    player_bank->money = 0;
  return 0;
}