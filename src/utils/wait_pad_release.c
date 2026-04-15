#include <gb/gb.h>

void wait_pad_release(void) {
  while (joypad()) {
    wait_vbl_done();
  }
}
