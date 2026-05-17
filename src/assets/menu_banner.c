#include <gb/gb.h>
#include <stdint.h>

#include "menu_banner.h"

const uint8_t banner_light_sprites[48] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x7E, 0x00, 0xFF, 0x00, 0xFF,
    0x00, 0xFF, 0x00, 0xFF, 0x00, 0x7E, 0x00, 0x3C, 0x3C, 0x3C, 0x7E, 0x7E,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7E, 0x7E, 0x3C, 0x3C,
};

static const uint8_t light_x[10] = {8, 24, 40, 56, 72, 88, 104, 120, 136, 152};

static uint8_t frame_counter = 0;

static uint8_t state_to_tile(uint8_t state) {
  if (state == 0)
    return SPRITE_LIGHT_ON;
  if (state == 2)
    return SPRITE_LIGHT_OFF;
  return SPRITE_LIGHT_DIM;
}

void load_banner_sprites(void) { set_sprite_data(0, 3, banner_light_sprites); }

void init_banner_lights(void) {
  uint8_t i;

  frame_counter = 0;

  for (i = 0; i < 10; i++) {
    move_sprite(i, light_x[i], 16);
    set_sprite_tile(i, (i & 1) ? SPRITE_LIGHT_OFF : SPRITE_LIGHT_ON);
    move_sprite(10 + i, light_x[i], 32);
    set_sprite_tile(10 + i, (i & 1) ? SPRITE_LIGHT_ON : SPRITE_LIGHT_OFF);
  }

  move_sprite(20, 8, 24);
  set_sprite_tile(20, SPRITE_LIGHT_ON);

  move_sprite(21, 152, 24);
  set_sprite_tile(21, SPRITE_LIGHT_OFF);
}

void update_banner_lights(void) {
  uint8_t i;
  uint8_t anim_step;
  uint8_t even_tile, odd_tile;

  frame_counter++;
  anim_step = (frame_counter >> 3) & 3;

  even_tile = state_to_tile(anim_step);
  odd_tile = state_to_tile((anim_step + 2) & 3);

  for (i = 0; i < 10; i++) {
    set_sprite_tile(i, (i & 1) ? odd_tile : even_tile);
    set_sprite_tile(10 + i, (i & 1) ? even_tile : odd_tile);
  }

  set_sprite_tile(20, even_tile);
  set_sprite_tile(21, odd_tile);
}
