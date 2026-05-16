#include <gb/gb.h>
#include <stdint.h>

#include "casino.h"
#include "games/games.h"
#include "init/init.h"
#include "save/save.h"
#include "utils/utils.h"
#include "assets/load_screen.h"

uint8_t luck;
uint8_t wave_counter = 0;
uint8_t delay_counter = 0;

const uint8_t wave_sinus[32] = {
    0, 1, 2, 3, 4, 4, 3, 2, 1, 0, 
    255, 254, 253, 252, 252, 253, 254, 255,
    0, 1, 2, 3, 4, 4, 3, 2, 1, 0,
    255, 254, 253, 252
};

void interupt_lcd(void)
{
    SCX_REG = wave_sinus[(LY_REG + wave_counter) & 31];
}

void draw_wave(void)
{
    disable_interrupts();
    add_LCD(interupt_lcd);
    STAT_REG |= STATF_MODE00; 
    set_interrupts(VBL_IFLAG | LCD_IFLAG);
    enable_interrupts(); 
}

void main(void)
{
    bank_t bank = {.name = "Player", .name_size = 6, .money = 0};
    uint8_t game_idx = 0;

    BGP_REG = 0xE4U;

    set_bkg_data(0, load_screen_TILE_COUNT, load_screen_tiles);
    set_bkg_tiles(0, 0, 20, 18, load_screen_map);

    SHOW_BKG;
    DISPLAY_ON;

    draw_wave();

    while (!(joypad() & J_START)) {
        delay_counter++;
        if (delay_counter >= 4) {
            wave_counter++;
            delay_counter = 0;
        }
        wait_vbl_done();
    }

    disable_interrupts();
    remove_LCD(interupt_lcd);
    SCX_REG = 0;
    set_interrupts(VBL_IFLAG);
    enable_interrupts();

    init();
    init_font();
    save_load(bank.name, &bank);

    draw_text(0, 0, bank.name);

    for (;;) {
        game_idx = game_tab[game_idx].game(&bank);
        save_write(&bank);
        wait_vbl_done();
    }
}