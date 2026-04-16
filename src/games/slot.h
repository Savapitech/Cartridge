
#ifndef SLOT_H
#define SLOT_H

#include <stdint.h>

#define T_HZ 96
#define T_VT 97
#define T_TL 98
#define T_TR 99
#define T_BL 100
#define T_BR 101
#define T_SP 0

typedef struct slot_s {
  uint8_t x;
  uint8_t y;
  uint8_t type;
} slot_t;

#define TARGET_Y 96
#define WIN 1
#define LOOSE 0

#endif
