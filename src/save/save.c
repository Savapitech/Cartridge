#include <gb/gb.h>
#include <stdint.h>
#include <string.h>

#include "save.h"

static volatile uint8_t *slot_addr(uint8_t slot) {
  return (volatile uint8_t *)(0xA000u + (uint16_t)slot * SLOT_BYTES);
}

static void read_raw(uint8_t slot, save_slot_t *out) {
  volatile uint8_t *p = slot_addr(slot);
  uint8_t *dst = (uint8_t *)out;
  uint8_t i;
  for (i = 0; i < SLOT_BYTES; i++)
    dst[i] = p[i];
}

static void write_raw(uint8_t slot, const save_slot_t *in) {
  volatile uint8_t *p = slot_addr(slot);
  const uint8_t *src = (const uint8_t *)in;
  uint8_t i;
  for (i = 0; i < SLOT_BYTES; i++)
    p[i] = src[i];
}

static uint8_t find_slot(const char *name) {
  save_slot_t s;
  uint8_t i;
  for (i = 0; i < MAX_SAVES; i++) {
    read_raw(i, &s);
    if (s.magic == SAVE_MAGIC && strncmp(s.name, name, 10) == 0)
      return i;
  }
  return SAVE_NONE;
}

static uint8_t first_free(void) {
  save_slot_t s;
  uint8_t i;
  for (i = 0; i < MAX_SAVES; i++) {
    read_raw(i, &s);
    if (s.magic != SAVE_MAGIC)
      return i;
  }
  return SAVE_NONE;
}

uint8_t save_find(const char *name) {
  uint8_t r;
  ENABLE_RAM;
  r = find_slot(name);
  DISABLE_RAM;
  return r;
}

uint8_t save_load(const char *name, bank_t *bank) {
  save_slot_t s;
  uint8_t slot;
  ENABLE_RAM;
  slot = find_slot(name);
  if (slot != SAVE_NONE) {
    read_raw(slot, &s);
    strncpy(bank->name, s.name, 10);
    bank->name[10] = '\0';
    bank->name_size = (uint8_t)strlen(bank->name);
    bank->money = s.money;
  }
  DISABLE_RAM;
  return (slot != SAVE_NONE);
}

void save_write(const bank_t *bank) {
  save_slot_t s;
  uint8_t slot;
  ENABLE_RAM;
  slot = find_slot(bank->name);
  if (slot == SAVE_NONE)
    slot = first_free();
  if (slot != SAVE_NONE) {
    s.magic = SAVE_MAGIC;
    strncpy(s.name, bank->name, 10);
    s.name[10] = '\0';
    s.money = bank->money;
    write_raw(slot, &s);
  }
  DISABLE_RAM;
}

uint8_t save_read_slot(uint8_t slot, save_slot_t *out) {
  ENABLE_RAM;
  read_raw(slot, out);
  DISABLE_RAM;
  return (out->magic == SAVE_MAGIC);
}
