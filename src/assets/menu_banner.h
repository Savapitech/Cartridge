#ifndef MENU_BANNER_H
#define MENU_BANNER_H

#include <stdint.h>

#define BANNER_ROWS 3

#define SPRITE_LIGHT_ON 0
#define SPRITE_LIGHT_DIM 1
#define SPRITE_LIGHT_OFF 2

void load_banner_sprites(void);
void init_banner_lights(void);
void update_banner_lights(void);

#endif
