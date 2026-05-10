#include <gb/gb.h>
#include <stdint.h>

#include "transitions.h"

void transition_wipe_down(void) {
  uint8_t y, x;
  for (y = 0; y < 18; y++) {
    for (x = 0; x < 20; x++)
      set_bkg_tile_xy(x, y, 0);
    wait_vbl_done();
  }
}

void transition_wipe_up(void) {
  int8_t y;
  uint8_t x;
  for (y = 17; y >= 0; y--) {
    for (x = 0; x < 20; x++)
      set_bkg_tile_xy(x, (uint8_t)y, 0);
    wait_vbl_done();
  }
}

void transition_flash(uint8_t times) {
  uint8_t i;
  for (i = 0; i < times; i++) {
    BGP_REG  = 0x00;
    OBP0_REG = 0x00;
    delay(55);
    BGP_REG  = 0xE4;
    OBP0_REG = 0xE4;
    delay(55);
  }
}

void transition_shake(void) {
  uint8_t i;
  for (i = 0; i < 6; i++) {
    SCX_REG = (i & 1) ? 2 : 254;
    delay(40);
  }
  SCX_REG = 0;
}
