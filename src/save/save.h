#ifndef SAVE_H
#define SAVE_H

#include <stdint.h>

#include "../casino.h"

#define SAVE_MAGIC 0xCA
#define MAX_SAVES 8
#define SAVE_NONE 0xFF
#define SLOT_BYTES 16

typedef struct {
  uint8_t magic;
  char name[11];
  uint32_t money;
} save_slot_t;

uint8_t save_find(const char *name);
uint8_t save_load(const char *name, bank_t *bank);
void save_write(const bank_t *bank);
uint8_t save_read_slot(uint8_t slot, save_slot_t *out);

#endif
