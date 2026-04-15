#include "../casino.h"
#include "utils.h"
#include "get_input.h"

#include <gb/gb.h>


const uint8_t coordAlpha[3] = {9, 18, 27};

void input_handler(char buffer[INPUT_BUFFER]) {
  uint8_t keys = 0;
  uint8_t car = 0;

  uint8_t cursor = 0;

  uint8_t coord = 0;

  draw_text(ALPHABET_PADDING, ALPHABET_COORD1, ALPHABET1);
  draw_text(ALPHABET_PADDING, ALPHABET_COORD2, ALPHABET2);
  draw_text(ALPHABET_PADDING, ALPHABET_COORD3, ALPHABET3);

  while (!(keys & J_A)) {
    keys = joypad();

    if ((keys & J_RIGHT) && coord < END - 1) {
      coord++;
    }
    if ((keys & J_LEFT) && coord > START) {
      coord--;
    }
    if ((keys & J_DOWN) && coord + 9 < END) {
      coord += 9;
    }
    if ((keys & J_UP) && coord >= 9) {
      coord -= 9;
    }

    if (coord >= coordAlpha[1]) {
      draw_text((coord - coordAlpha[1] - 1) << 1, ALPHABET_COORD3, " ");
      draw_text((coord - coordAlpha[1] + 1) << 1, ALPHABET_COORD3, " ");
      draw_text(coord - coordAlpha[1] << 1, ALPHABET_COORD3 + 1, " ");
      draw_text(coord - coordAlpha[1] << 1, ALPHABET_COORD3 - 1, " ");
      draw_text((coord - coordAlpha[1]) << 1, ALPHABET_COORD3, ">");

    } else if (coord >= coordAlpha[0]) {
      draw_text((coord - coordAlpha[0] - 1) << 1, ALPHABET_COORD2, " ");
      draw_text((coord - coordAlpha[0] + 1) << 1, ALPHABET_COORD2, " ");
      draw_text(coord - coordAlpha[0] << 1, ALPHABET_COORD2 + 1, " ");
      draw_text(coord - coordAlpha[0] << 1, ALPHABET_COORD2 - 1, " ");
      draw_text((coord - coordAlpha[0]) << 1, ALPHABET_COORD2, ">");
    } else {
      draw_text((coord - 1) << 1, ALPHABET_COORD1, " ");
      draw_text((coord + 1) << 1, ALPHABET_COORD1, " ");
      draw_text((coord) << 1, ALPHABET_COORD1 + 1, " ");
      draw_text((coord) << 1, ALPHABET_COORD1 - 1, " ");
      draw_text(coord << 1, ALPHABET_COORD1, ">");
    }

    if (keys & J_B) {
      if (coord == END - 1) {
        cursor = (cursor == 0) ? (cursor) : (cursor - 1);
        buffer[cursor] = '_';
      } else {
        buffer[cursor] = coord + 'a';
        cursor++;
      }
      if (cursor == 10)
        return;
    }
    draw_text(INPUT_HANDLER_COORD_X, INPUT_HANDLER_COORD_Y, buffer);
    wait_pad_release();
    wait_vbl_done();
  }
  buffer[cursor] = '\0';
  return;
}
