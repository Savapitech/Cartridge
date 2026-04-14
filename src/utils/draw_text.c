#include <stdint.h>
#include <gb/gb.h>


void draw_text(uint8_t x, uint8_t y, const char *str)
{
    uint8_t i = 0;

    while (str[i])
    {
        set_win_tile_xy(x + i, y, str[i] - 32);
        i++;
    }
}