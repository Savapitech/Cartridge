#include <gb/gb.h>
#include <gbdk/console.h>
#include <stdio.h>
#include <string.h>
#include <gb/gb.h>
#include <gbdk/font.h>
#include "utils/utils.h"

#include "casino.h"

bank_t bank = {
    .name = "Arthur",
    .name_size = 6,
    .money = 4242
};

uint8_t slot_machine(bank_t *player_bank) {
    return 1;
}

uint8_t roulette(bank_t *player_bank) {
    return 0;
}

void init(void)
{
    DISPLAY_ON;

    SHOW_BKG;
    SHOW_WIN;
    SHOW_SPRITES;

    BGP_REG = 0xE4;
    OBP0_REG = 0xD2;

    SCX_REG = 0;
    SCY_REG = 0;
}

void init_font(void)
{
    font_init();

    font_t font = font_load(font_ibm);
    font_set(font);
}


void render(bank_t *bank)
{
    draw_text(0, 0, bank->name);
    draw_money(bank->money, 0, 1);
}



const game_t game_tab[] = {
    { "Slots",    5, slot_machine },
    { "Roulette", 8, roulette }
};

void main(void)
{
    init();
    init_font();

    draw_text(0, 0, bank.name);

    while(1)
    {
        render_money(&bank);
        wait_vbl_done();        
    }
}