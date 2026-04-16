#include "get_input.h"
#include "../casino.h"
#include "utils.h"

#include <gb/gb.h>

const uint8_t coordAlpha[3] = {9, 18, 27};

static uint8_t get_cursor_x(uint8_t c) {
  if (c >= coordAlpha[1])
    return (c - coordAlpha[1]) << 1;
  if (c >= coordAlpha[0])
    return (c - coordAlpha[0]) << 1;
  return c << 1;
}

static uint8_t get_cursor_y(uint8_t c) {
  if (c >= coordAlpha[1])
    return ALPHABET_COORD3;
  if (c >= coordAlpha[0])
    return ALPHABET_COORD2;
  return ALPHABET_COORD1;
}

void input_handler(char buffer[INPUT_BUFFER]) {
  uint8_t keys = 0;
  uint8_t cursor = 0;
  uint8_t coord = 0;
  uint8_t last_coord = 255;
  draw_text(ALPHABET_PADDING, ALPHABET_COORD1, ALPHABET1);
  draw_text(ALPHABET_PADDING, ALPHABET_COORD2, ALPHABET2);
  draw_text(ALPHABET_PADDING, ALPHABET_COORD3, ALPHABET3);
  draw_text(INPUT_HANDLER_COORD_X, INPUT_HANDLER_COORD_Y, buffer);

  while (!(keys & J_A)) {
    keys = joypad();

    if ((keys & J_RIGHT) && coord < END - 1) {
      coord++;
      wait_pad_release();
    } else if ((keys & J_LEFT) && coord > START) {
      coord--;
      wait_pad_release();
    } else if ((keys & J_DOWN) && coord + 9 < END) {
      coord += 9;
      wait_pad_release();
    } else if ((keys & J_UP) && coord >= 9) {
      coord -= 9;
      wait_pad_release();
    }

    if (keys & J_B) {
      if (coord == END - 1) {
        if (cursor > 0)
          cursor--;
        buffer[cursor] = '_';
      } else {
        buffer[cursor] = coord + 'a';
        cursor++;
      }
      draw_text(INPUT_HANDLER_COORD_X, INPUT_HANDLER_COORD_Y, buffer);
      wait_pad_release();

      if (cursor == 10) {
        buffer[cursor] = '\0';
        return;
      }
    }

    if (coord != last_coord) {
      if (last_coord != 255) {
        draw_text(get_cursor_x(last_coord), get_cursor_y(last_coord), " ");
      }
      draw_text(get_cursor_x(coord), get_cursor_y(coord), ">");
      last_coord = coord;
    }

    wait_vbl_done();
  }
}