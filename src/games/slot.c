#include <gb/gb.h>
#include <rand.h>

#include "../casino.h"
#include "../utils/utils.h"
#include "games.h"
#include "menu.h"
#include "slot.h"

#include "../assets/slot_asset.h"

const uint8_t slot_pos[4]    = {58, 77, 94, 110};
const uint8_t bet_options[4] = {5, 10, 25, 50};
slot_t slots_array[16];

const uint8_t slot_machine_border[] = {
    T_TL, T_HZ, T_HZ, T_HZ, T_HZ, T_HZ, T_HZ, T_HZ, T_HZ, T_TR,
    T_VT, T_SP, T_SP, T_SP, T_SP, T_SP, T_SP, T_SP, T_SP, T_VT,
    T_BL, T_HZ, T_HZ, T_HZ, T_HZ, T_HZ, T_HZ, T_HZ, T_HZ, T_BR};

void play_jackpot(void)
{
    uint8_t i;
    NR21_REG = 0x80; NR22_REG = 0xF3;
    NR23_REG = 0x40; NR24_REG = 0xC3;
    delay(90);
    NR23_REG = 0x70; NR24_REG = 0xC3;
    delay(90);
    NR23_REG = 0x90; NR24_REG = 0xC3;
    delay(90);
    for (i = 0; i < 3; i++) {
        NR21_REG = 0x82; NR22_REG = 0xF1;
        NR23_REG = 0x55 + i * 0x10; NR24_REG = 0xC3;
        delay(60);
    }
    NR21_REG = 0x80; NR22_REG = 0xF1;
    NR23_REG = 0xA0; NR24_REG = 0xC5;
    delay(300);
}

void play_loose(void)
{
    NR21_REG = 0x80; NR22_REG = 0xF2;
    NR23_REG = 0x70; NR24_REG = 0xC3;
    delay(80);
    NR23_REG = 0x50; NR24_REG = 0xC3;
    delay(80);
    NR23_REG = 0x30; NR24_REG = 0xC3;
    delay(120);
}

void play_spin_tick(void)
{
    NR41_REG = 0x3F;
    NR42_REG = 0x51;
    NR43_REG = 0x28;
    NR44_REG = 0x80;
}

void play_stop_clack(void)
{
    NR41_REG = 0x20;
    NR42_REG = 0x72;
    NR43_REG = 0x16;
    NR44_REG = 0x80;
    delay(40);
    NR41_REG = 0x20;
    NR42_REG = 0x61;
    NR43_REG = 0x26;
    NR44_REG = 0x80;
}

void play_near_miss(void)
{
    NR21_REG = 0x80; NR22_REG = 0xB2;
    NR23_REG = 0x60; NR24_REG = 0xC3;
    delay(120);
    NR21_REG = 0x80; NR22_REG = 0x82;
    NR23_REG = 0x45; NR24_REG = 0xC3;
    delay(150);
}

void play_select_bip(void)
{
    NR21_REG = 0x82; NR22_REG = 0x93;
    NR23_REG = 0x80; NR24_REG = 0xC5;
}

void play_confirm(void)
{
    NR21_REG = 0x80; NR22_REG = 0xA2;
    NR23_REG = 0x70; NR24_REG = 0xC4;
    delay(60);
    NR21_REG = 0x80; NR22_REG = 0xA1;
    NR23_REG = 0x90; NR24_REG = 0xC4;
    delay(80);
}


void slot_flash(uint8_t times)
{
    uint8_t i;
    for (i = 0; i < times; i++) {
        BGP_REG  = 0x00;
        OBP0_REG = 0x00;
        delay(55);
        BGP_REG  = 0xE4;
        OBP0_REG = 0xE4;
        delay(55);
    }
}

void slot_shake(void)
{
    uint8_t i;
    for (i = 0; i < 5; i++) {
        SCX_REG = (i & 1) ? 3 : 253;
        delay(35);
    }
    SCX_REG = 0;
}

void slot_wipe(void)
{
    int8_t y;
    uint8_t x;
    for (y = 17; y >= 0; y--) {
        for (x = 0; x < 20; x++)
            set_bkg_tile_xy(x, (uint8_t)y, 0);
        wait_vbl_done();
    }
}

void win_animation(void)
{
    uint8_t y = 0;
    uint8_t coin_x[5] = {20, 50, 80, 110, 140};
    uint8_t coin_y[5] = {0, 220, 240, 210, 230};
    uint8_t i;

    for (i = 0; i < 5; i++)
        set_sprite_tile(i, 2);

    play_jackpot();

    while (y != 120) {
        if ((y >> 2) & 1) {
            HIDE_SPRITES; HIDE_BKG;
        } else {
            SHOW_SPRITES; SHOW_BKG;
        }

        for (i = 0; i < 5; i++) {
            coin_y[i] += 4;
            if (coin_y[i] > 160 && coin_y[i] < 200)
                coin_y[i] = 0;
            move_sprite(i, coin_x[i], coin_y[i]);
        }

        if ((y & 15) == 0) {
            NR21_REG = 0x80; NR22_REG = 0x82;
            NR23_REG = 0x80; NR24_REG = 0xC7;
        }

        wait_vbl_done();
        y++;
    }
    SHOW_SPRITES;
    SHOW_BKG;
}

uint8_t choose_bet_slot(bank_t *player_bank)
{
    uint8_t selected = 1;
    uint8_t keys = 0, prev_keys, keys_pressed;
    uint8_t i;

    {
        uint8_t s;
        for (s = 0; s < 40; s++)
            move_sprite(s, 0, 0);
    }
    HIDE_SPRITES;

    slot_wipe();
    CLEAR_BKG;

    draw_text(0, 0, player_bank->name);
    draw_money(player_bank->money, 0, 1);
    draw_text(2, 3, "* SLOT MACHINE *");
    draw_text(2, 5, "PLACE YOUR BET");
    draw_text(1, 6, "UP/DN:SELECT  A:GO");

    while (bet_options[selected] > player_bank->money && selected > 0)
        selected--;

    while (1) {
        for (i = 0; i < BET_OPTIONS_COUNT; i++) {
            uint8_t row = 9 + i;
            if (bet_options[i] > player_bank->money) {
                draw_text(3, row, "-- (NO FUNDS)  ");
            } else {
                if (i == selected)
                    draw_text(2, row, "> BET:");
                else
                    draw_text(2, row, "  BET:");
                draw_money(bet_options[i], 9, row);
            }
        }

        draw_text(2, 15, "WIN = BET x 10");

        prev_keys = keys;
        keys = joypad();
        keys_pressed = (keys ^ prev_keys) & keys;

        if (keys_pressed & J_SELECT)
            return 0;
        if (keys_pressed & J_UP) {
            int8_t next = (int8_t)selected - 1;
            while (next >= 0 && bet_options[next] > player_bank->money)
                next--;
            if (next >= 0) {
                selected = (uint8_t)next;
                play_select_bip();
            }
        }
        if (keys_pressed & J_DOWN) {
            uint8_t next = selected + 1;
            while (next < BET_OPTIONS_COUNT && bet_options[next] > player_bank->money)
                next++;
            if (next < BET_OPTIONS_COUNT) {
                selected = next;
                play_select_bip();
            }
        }
        if (keys_pressed & J_A) {
            if (bet_options[selected] <= player_bank->money) {
                play_confirm();
                return bet_options[selected];
            }
        }
        wait_vbl_done();
    }
}

void init_slot(void)
{
    uint8_t i;
    slot_t *ptr;

    CLEAR_BKG;
    NR52_REG = 0x80;
    NR51_REG = 0xFF;
    NR50_REG = 0x77;

    set_sprite_data(0, 1, star_tile);
    set_sprite_data(1, 1, cross_tile);
    set_sprite_data(2, 1, circle_tile);
    set_sprite_data(3, 1, epitech_tile);

    ptr = slots_array;
    for (i = 0; i < 16; i++) {
        set_sprite_tile(i, 0);
        ptr->x    = slot_pos[i >> 2];
        ptr->y    = 80 + ((i & 3) << 4);
        ptr->type = (uint8_t)rand() & 3;
        set_sprite_tile(i, ptr->type);
        if (ptr->y < TARGET_Y - 2 || ptr->y > TARGET_Y + 4)
            move_sprite(i, 0, 0);
        else
            move_sprite(i, ptr->x, ptr->y);
        ptr++;
    }

    draw_text(4, 5, "SLOT MACHINE");
    set_bkg_tiles(5, 9, 10, 3, slot_machine_border);
    SHOW_SPRITES;
}

uint8_t check_win(void)
{
    uint8_t winning_types[4];
    uint8_t found = 0;
    uint8_t i;

    for (i = 0; i < 16; i++) {
        if (slots_array[i].y == TARGET_Y) {
            winning_types[found] = slots_array[i].type;
            found++;
        }
    }

    if (found != 4)
        return LOOSE;

    if (winning_types[0] == winning_types[1] &&
        winning_types[1] == winning_types[2] &&
        winning_types[2] == winning_types[3]) {
        return WIN;
    }
    return LOOSE;
}

uint8_t check_near_win(void)
{
    uint8_t types[3];
    uint8_t found = 0;
    uint8_t i;
    for (i = 0; i < 12; i++) {
        if (slots_array[i].y == TARGET_Y) {
            types[found] = slots_array[i].type;
            found++;
        }
    }
    if (found == 3 && types[0] == types[1] && types[1] == types[2])
        return 1;
    return 0;
}

void stop_slot(uint8_t col_start)
{
    uint8_t i;
    slot_t *col = &slots_array[col_start];
    uint8_t best_idx = 0;
    uint8_t min_dist = 255;

    for (i = 0; i < 4; i++) {
        uint8_t dist = (col[i].y > TARGET_Y) ? (col[i].y - TARGET_Y)
                                              : (TARGET_Y - col[i].y);
        if (dist < min_dist) {
            min_dist = dist;
            best_idx = i;
        }
    }

    while (col[best_idx].y != TARGET_Y) {
        int8_t step = (col[best_idx].y < TARGET_Y) ? 1 : -1;
        for (i = 0; i < 4; i++) {
            col[i].y += step;
            if (col[i].y >= 144)
                col[i].y = 80;
            else if (col[i].y < 80)
                col[i].y = 143;
            if (col[i].y < TARGET_Y - 2 || col[i].y > TARGET_Y + 4)
                move_sprite(col_start + i, 0, 0);
            else
                move_sprite(col_start + i, col[i].x, col[i].y);
        }
        wait_vbl_done();
    }

    play_stop_clack();

    for (i = 0; i < 4; i++) {
        if (i != best_idx) {
            col[i].y = 0;
            move_sprite(col_start + i, 0, 0);
        }
    }
}

uint8_t slot_machine(bank_t *player_bank)
{
    uint8_t keys = 0, prev_keys = 0, keys_pressed = 0;
    uint8_t stop_col  = 0;
    uint8_t cooldown  = 0;
    uint8_t target_type = 255;
    uint8_t spin_tick = 0;
    uint8_t bet;

    bet = choose_bet_slot(player_bank);
    if (bet == 0) 
        return MENU;

    player_bank->money -= bet;

    init_slot();
    draw_text(0, 0, player_bank->name);
    draw_money(player_bank->money, 0, 1);

    draw_text(0, 2, "BET:");
    draw_money(bet, 5, 2);
    draw_text(0, 3, "B:STOP COL  SEL:QUIT");
    while (!(keys & J_SELECT)) {
        slot_t *ptr;
        uint8_t i;

        prev_keys = keys;
        keys = joypad();
        keys_pressed = (keys ^ prev_keys) & keys;

        if (cooldown > 0) 
            cooldown--;
        spin_tick++;
        if (spin_tick >= 12 && stop_col < 16) {
            play_spin_tick();
            spin_tick = 0;
        }

        if ((keys_pressed & J_B) && (cooldown == 0)) {
            if (stop_col < 16) {
                stop_slot(stop_col);
                if (stop_col == 0) {
                    for (i = 0; i < 4; i++) {
                        if (slots_array[i].y == TARGET_Y) {
                            target_type = slots_array[i].type;
                            break;
                        }
                    }
                }

                stop_col += 4;
                cooldown = 15;
            }
            if (stop_col == 16) {
                uint8_t near = check_near_win();
                uint8_t won  = check_win();

                delay(400);

                if (won) {
                    uint8_t winnings = bet * 10;
                    slot_flash(5);
                    player_bank->money += winnings;
                    draw_money(player_bank->money, 0, 1);
                    draw_text(1, 14, "JACKPOT! x10    ");
                    win_animation();
                } else {
                    if (near) {
                        slot_shake();
                        play_near_miss();
                        draw_text(1, 14, "SO CLOSE...!    ");
                        delay(600);
                    } else {
                        play_loose();
                        draw_text(1, 14, "NO LUCK...      ");
                    }
                    delay(800);
                }
                if (player_bank->money == 0) {
                    CLEAR_BKG;
                    draw_text(2, 8,  "GAME OVER!");
                    draw_text(1, 10, "NO MORE FUNDS");
                    delay(2000);
                    return MENU;
                }

                return SLOT;
            }
        }
        ptr = slots_array + stop_col;
        for (i = stop_col; i < 16; i++) {
            ptr->y += SPIN_TICK_RATE;
            if (ptr->y >= 144) {
                if (target_type != 255 && ((uint8_t)rand() & 1) == 0) {
                    ptr->type = target_type;
                } else {
                    ptr->type = (uint8_t)rand() & 3;
                }
                set_sprite_tile(i, ptr->type);
                ptr->y = 80;
            }

            if (ptr->y < TARGET_Y - 2 || ptr->y > TARGET_Y + 4)
                move_sprite(i, 0, 0);
            else
                move_sprite(i, ptr->x, ptr->y);
            ptr++;
        }

        wait_vbl_done();
    }
    player_bank->money += bet;
    return MENU;
}