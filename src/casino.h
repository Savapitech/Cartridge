#ifndef CASINO_H
#define CASINO_H

#include <gb/gb.h>
#include <stdint.h>

#define CLEAR_WIN fill_win_rect(0, 0, 20, 32, 0)

typedef struct bank_s {
  char name[16];
  uint8_t name_size;
  uint32_t money;
} bank_t;

#endif