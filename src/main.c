#include <gb/gb.h>
#include <stdlib.h>
#include <stdio.h>

#include "asset/sonic.h"


uint8_t position_X = 8;
uint8_t position_Y = 16;

void main(void) {
    printf("Hello World!");

    DISPLAY_ON;
    SHOW_SPRITES;
    SHOW_BKG;
    SPRITES_8x8;

    set_sprite_data(0, 1, sonic_tiles);
    set_sprite_tile(0, 0);
    
    move_sprite(0, position_X, position_Y);

    while(1) {
        uint8_t joy = joypad();

        if (joy & J_LEFT) {
            position_X--;
        }
        if (joy & J_RIGHT) {
            position_X++;
        }
        if (joy & J_UP) {
            position_Y--;
        }
        if (joy & J_DOWN) {
            position_Y++;
        }
        move_sprite(0, position_X, position_Y);
        wait_vbl_done(); 
    }
}
