#ifndef SLOT_H
#define SLOT_H

#include <stdint.h>

#define SLOT_SPR_BASE 200

#define SLOT_BRD_TL 113
#define SLOT_BRD_TR 114
#define SLOT_BRD_BL 115
#define SLOT_BRD_BR 116
#define SLOT_BRD_H 117
#define SLOT_BRD_HB 118
#define SLOT_BRD_VL 119
#define SLOT_BRD_VR 120

#define BET_OPTIONS_COUNT 4
#define SPIN_TICK_RATE 2

typedef struct slot_s {
  uint8_t x;
  uint8_t y;
  uint8_t type;
} slot_t;

#define TARGET_Y 96
#define WIN 1
#define LOOSE 0

#endif
