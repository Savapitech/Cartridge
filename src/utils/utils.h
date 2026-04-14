#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include "../casino.h"

void draw_text(uint8_t x, uint8_t y, const char *str);
void draw_money(uint32_t money, uint8_t x, uint8_t y);
void render_money(bank_t *player_bank);

#endif