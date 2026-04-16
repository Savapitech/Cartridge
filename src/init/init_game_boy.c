#include <gb/gb.h>

void init(void) {
  DISPLAY_ON;

  SHOW_BKG;
  SHOW_WIN;

  BGP_REG = 0xE4;
  OBP0_REG = 0xD2;

  SCX_REG = 0;
  SCY_REG = 0;
}