#include <gbdk/font.h>

void init_font(void) {
  font_init();

  font_t font = font_load(font_ibm);
  font_set(font);
}