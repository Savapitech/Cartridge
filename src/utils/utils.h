#ifndef UTILS_H
#define UTILS_H

#include "../casino.h"
#include "get_input.h"
#include <stdint.h>

void load_card_at(uint8_t x, uint8_t y, uint8_t val, uint8_t suit);
void reset_card_memory(void);
void draw_text(uint8_t x, uint8_t y, const char *str);
void draw_money(uint32_t money, uint8_t x, uint8_t y);
void render_money(bank_t *player_bank);
void wait_pad_release(void);
void input_handler(char buffer[INPUT_BUFFER]);

#endif