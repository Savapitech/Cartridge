#ifndef CASINO_H
#define CASINO_H

#include <stdint.h>

typedef struct bank_s {
  char name[16];
  uint8_t name_size;
  uint32_t money;
} bank_t;

#endif