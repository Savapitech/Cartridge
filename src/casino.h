#ifndef CASINO_H
#define CASINO_H

#include <stdint.h>
#include <gb/gb.h>

#define CLEAR_WIN fill_win_rect(0, 0, 20, 32, 0)

typedef struct bank_s {
  char name[16];
  uint8_t name_size;
  uint32_t money;
} bank_t;

#endif