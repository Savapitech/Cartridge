#ifndef UTILS_H
#define UTILS_H

#include "../casino.h"
#include <stdint.h>
#include "get_input.h"

void draw_text(uint8_t x, uint8_t y, const char *str);
void draw_money(uint32_t money, uint8_t x, uint8_t y);
void render_money(bank_t *player_bank);
void wait_pad_release(void);
void input_handler(char buffer[INPUT_BUFFER]);

#endif