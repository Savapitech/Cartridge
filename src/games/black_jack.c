#include <gb/gb.h>
#include <gb/cgb.h>
#include <rand.h>

#include "../casino.h"
#include "../utils/utils.h"
#include "games.h"
#include "menu.h"
#include "black_jack.h"

#include "../assets/black_jack_asset.h"

void audio_init(void)
{
    NR52_REG = 0x80;
    NR50_REG = 0x77;
    NR51_REG = 0xFF;
}

static void ch1_play(uint8_t freq_lo, uint8_t freq_hi,
                     uint8_t duty, uint8_t len,
                     uint8_t env_start, uint8_t env_dir, uint8_t env_step)
{
    NR11_REG = (duty << 6) | (64 - len);
    NR12_REG = (env_start << 4) | (env_dir << 3) | env_step;
    NR13_REG = freq_lo;
    NR14_REG = 0x80 | (len ? 0x40 : 0x00) | (freq_hi & 0x07);
}

static void ch2_play(uint8_t freq_lo, uint8_t freq_hi,
                     uint8_t duty, uint8_t len,
                     uint8_t env_start, uint8_t env_dir, uint8_t env_step)
{
    NR21_REG = (duty << 6) | (64 - len);
    NR22_REG = (env_start << 4) | (env_dir << 3) | env_step;
    NR23_REG = freq_lo;
    NR24_REG = 0x80 | (len ? 0x40 : 0x00) | (freq_hi & 0x07);
}

static void ch4_play(uint8_t env_start, uint8_t env_dir, uint8_t env_step,
                     uint8_t shift, uint8_t width, uint8_t len)
{
    NR41_REG = 64 - len;
    NR42_REG = (env_start << 4) | (env_dir << 3) | env_step;
    NR43_REG = (shift << 4) | (width << 3);
    NR44_REG = 0x80 | (len ? 0x40 : 0x00);
}

void sfx_card_flip(void)
{
    ch4_play(8, 0, 1, 5, 0, 32);
}

void sfx_hit(void)
{
    ch1_play(NOTE_G4_LO, NOTE_G4_HI, 2, 8, 12, 0, 3);
}

void sfx_stand(void)
{
    ch1_play(NOTE_E5_LO, NOTE_E5_HI, 1, 10, 10, 0, 4);
    delay(80);
    ch1_play(NOTE_C4_LO, NOTE_C4_HI, 1, 10,  8, 0, 4);
}

void sfx_win(void)
{
    ch1_play(NOTE_C4_LO, NOTE_C4_HI, 2, 12, 14, 0, 2); delay(120);
    ch1_play(NOTE_E4_LO, NOTE_E4_HI, 2, 12, 14, 0, 2); delay(120);
    ch1_play(NOTE_G4_LO, NOTE_G4_HI, 2, 12, 14, 0, 2); delay(120);
    ch1_play(NOTE_C5_LO, NOTE_C5_HI, 2, 20, 15, 0, 1);
}

void sfx_blackjack(void)
{
    uint8_t i;
    ch1_play(NOTE_C5_LO, NOTE_C5_HI, 2, 15, 15, 0, 1);
    ch2_play(NOTE_E5_LO, NOTE_E5_HI, 2, 15, 13, 0, 1); delay(150);
    ch1_play(NOTE_E5_LO, NOTE_E5_HI, 2, 15, 15, 0, 1);
    ch2_play(NOTE_G4_LO, NOTE_G4_HI, 2, 15, 13, 0, 1); delay(150);
    ch1_play(NOTE_G4_LO, NOTE_G4_HI, 2, 15, 15, 0, 1); delay(150);
    ch1_play(NOTE_C5_LO, NOTE_C5_HI, 2, 30, 15, 0, 1);
    ch2_play(NOTE_E5_LO, NOTE_E5_HI, 2, 30, 13, 0, 1); delay(300);
    for (i = 0; i < 3; i++) {
        ch1_play(NOTE_C5_LO, NOTE_C5_HI + i, 3, 8, 12, 0, 2);
        delay(80);
    }
}

void sfx_lose(void)
{
    ch1_play(NOTE_C4_LO, NOTE_C4_HI,     1, 12, 12, 0, 4); delay(100);
    ch1_play(NOTE_B4_LO, NOTE_B4_HI - 1, 1, 12, 10, 0, 4); delay(100);
    ch4_play(12, 0, 2, 3, 0, 30); delay(200);
    ch4_play(14, 1, 1, 3, 0, 40);
}

void sfx_push(void)
{
    ch1_play(NOTE_G4_LO, NOTE_G4_HI, 1, 12, 10, 0, 3); delay(120);
    ch1_play(NOTE_G4_LO, NOTE_G4_HI, 1, 12, 10, 0, 3);
}

void sfx_double_down(void)
{
    ch1_play(NOTE_E5_LO, NOTE_E5_HI, 3, 20, 15, 0, 1);
    ch2_play(NOTE_C5_LO, NOTE_C5_HI, 3, 20, 13, 0, 1); delay(200);
    ch1_play(NOTE_G4_LO, NOTE_G4_HI, 3, 15, 12, 0, 2);
}

void sfx_select(void)
{
    ch2_play(NOTE_E4_LO, NOTE_E4_HI, 2, 6,  8, 0, 5); 
}

void sfx_confirm(void)
{
    ch2_play(NOTE_G4_LO, NOTE_G4_HI, 2, 8, 10, 0, 3); delay(60);
    ch2_play(NOTE_C5_LO, NOTE_C5_HI, 2, 8, 12, 0, 2);
}

void sfx_game_over(void)
{
    ch1_play(NOTE_C5_LO, NOTE_C5_HI, 0, 30, 14, 0, 1); delay(250);
    ch1_play(NOTE_B4_LO, NOTE_B4_HI, 0, 30, 12, 0, 2); delay(250);
    ch1_play(NOTE_A4_LO, NOTE_A4_HI, 0, 30, 10, 0, 2); delay(250);
    ch1_play(NOTE_G4_LO, NOTE_G4_HI, 0, 40,  8, 0, 3);
    ch4_play(12, 0, 2, 4, 0, 40); delay(400);
    ch4_play(14, 1, 1, 5, 0, 40);
}

void transition_wipe_down(void)
{
    uint8_t y, x;
    for (y = 0; y < 18; y++) {
        for (x = 0; x < 20; x++) set_bkg_tile_xy(x, y, 0);
        wait_vbl_done();
    }
}

void transition_wipe_up(void)
{
    int8_t  y;
    uint8_t x;
    for (y = 17; y >= 0; y--) {
        for (x = 0; x < 20; x++) set_bkg_tile_xy(x, (uint8_t)y, 0);
        wait_vbl_done();
    }
}

void transition_flash(uint8_t times)
{
    uint8_t i;
    for (i = 0; i < times; i++) {
        BGP_REG = 0x00; delay(60);
        BGP_REG = 0xE4; delay(60);
    }
}

void transition_shake(void)
{
    uint8_t i;
    for (i = 0; i < 6; i++) {
        SCX_REG = (i & 1) ? 2 : 254;
        delay(40);
    }
    SCX_REG = 0;
}

const char card_chars[] = " A23456789TJQK";

void load_suits(void)
{
    set_bkg_data(128, 1, coeurs_tile);
    set_bkg_data(129, 1, pique_tile);
    set_bkg_data(130, 1, trefle_tile);
    set_bkg_data(131, 1, carreau_tile);
}

void draw_card(uint8_t x, uint8_t y, uint8_t val, uint8_t suit)
{
    set_bkg_tile_xy(x,     y,     card_chars[val] - 32);
    set_bkg_tile_xy(x + 1, y,     ' ' - 32);
    set_bkg_tile_xy(x,     y + 1, ' ' - 32);
    set_bkg_tile_xy(x + 1, y + 1, 128 + suit);
}

void draw_hidden_card(uint8_t x, uint8_t y)
{
    set_bkg_tile_xy(x,     y,     '?' - 32);
    set_bkg_tile_xy(x + 1, y,     '?' - 32);
    set_bkg_tile_xy(x,     y + 1, '?' - 32);
    set_bkg_tile_xy(x + 1, y + 1, '?' - 32);
}

void draw_card_animated(uint8_t x, uint8_t y, uint8_t val, uint8_t suit)
{
    sfx_card_flip();
    draw_card(x, y, val, suit);
    delay(80);
}

uint8_t calc_score(uint8_t *hand, uint8_t count)
{
    uint8_t score = 0, aces = 0, i;
    for (i = 0; i < count; i++) {
        uint8_t v = hand[i];
        if (v == 1) { 
            aces++;
            score += 11;
        } else if (v >= 10)
            score += 10; 
        else
            score += v;
    }
    while (score > 21 && aces > 0) {
        score -= 10;
        aces--; 
    }
    return score;
}

void draw_score(uint8_t score, uint8_t x, uint8_t y)
{
    uint8_t tens  = 0;
    uint8_t units = score;
    while (units >= 10) {
        units -= 10;
        tens++;
    }
    set_bkg_tile_xy(x, y, tens  ? ('0' + tens  - 32) : (' ' - 32));
    set_bkg_tile_xy(x + 1, y, '0' + units - 32);
}

uint8_t choose_bet(bank_t *player_bank)
{
    const uint8_t bet_options[] = {5, 10, 25, 50};
    const uint8_t num_options   = 4;
    uint8_t selected = 1;
    uint8_t keys = 0, prev_keys, keys_pressed;

    transition_wipe_down();
    draw_text(0, 0, player_bank->name);
    draw_money(player_bank->money, 0, 1);
    draw_text(3, 4, "PLACE YOUR BET");
    draw_text(2, 6, "UP/DOWN: SELECT");
    draw_text(2, 7, "A: CONFIRM");

    while (bet_options[selected] > player_bank->money && selected > 0)
        selected--;

    while (1) {
        uint8_t i;
        for (i = 0; i < num_options; i++) {
            uint8_t row = 9 + i;
            if (bet_options[i] > player_bank->money) {
                draw_text(3, row, "-- (NO FUNDS)");
            } else {
                draw_text(2, row, (i == selected) ? "> BET:" : "  BET:");
                draw_money(bet_options[i], 9, row);
            }
        }

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
                sfx_select(); 
            }
        }
        if (keys_pressed & J_DOWN) {
            uint8_t next = selected + 1;
            while (next < num_options && bet_options[next] > player_bank->money)
                next++;
            if (next < num_options) { 
                selected = next;
                sfx_select();
            }
        }
        if ((keys_pressed & J_A) && bet_options[selected] <= player_bank->money) {
            sfx_confirm();
            return bet_options[selected];
        }
        wait_vbl_done();
    }
}

uint8_t black_jack(bank_t *player_bank)
{
    uint8_t keys = 0, prev_keys = 0, keys_pressed = 0;
    uint8_t p_hand[10], p_suits[10], p_count;
    uint8_t d_hand[10], d_suits[10], d_count;
    uint8_t p_score, d_score, state, bet, doubled_down;

    audio_init();
    load_suits();

    while (1) {
        bet = choose_bet(player_bank);
        if (bet == 0)
            return MENU;
        transition_wipe_up();
        CLEAR_BKG;
        draw_text(0, 0, player_bank->name);
        draw_money(player_bank->money, 0, 1);
        draw_text(1, 2, "BET:");
        draw_money(bet, 6, 2);
        draw_text(1, 3, "DEALER");
        draw_text(1, 8, "PLAYER");

        p_count = d_count = state = doubled_down = 0;
        p_hand[p_count]  = ((uint8_t)rand() % 13) + 1;
        p_suits[p_count] = (uint8_t)rand() & 3;
        draw_card_animated(CARD_X(p_count), 10, p_hand[p_count], p_suits[p_count]);
        p_count++;

        d_hand[d_count]  = ((uint8_t)rand() % 13) + 1;
        d_suits[d_count] = (uint8_t)rand() & 3;
        draw_card_animated(CARD_X(d_count), 5, d_hand[d_count], d_suits[d_count]);
        d_count++;

        p_hand[p_count]  = ((uint8_t)rand() % 13) + 1;
        p_suits[p_count] = (uint8_t)rand() & 3;
        draw_card_animated(CARD_X(p_count), 10, p_hand[p_count], p_suits[p_count]);
        p_count++;

        d_hand[d_count]  = ((uint8_t)rand() % 13) + 1;
        d_suits[d_count] = (uint8_t)rand() & 3;
        sfx_card_flip();
        draw_hidden_card(CARD_X(d_count), 5);
        d_count++;

        p_score = calc_score(p_hand, p_count);
        draw_text(13, 8, "SC:");
        draw_score(p_score, 16, 8);
        if (p_score == 21)
            state = 3;
        wait_pad_release();
        if (state == 0) {
            draw_text(0, 14, ((bet << 1) <= player_bank->money)
                             ? "A:HIT B:STA ST:DBL"
                             : "A:HIT    B:STAND  ");
        }
        while (state == 0) {
            prev_keys = keys;
            keys = joypad();
            keys_pressed = (keys ^ prev_keys) & keys;

            if (keys_pressed & J_SELECT)
                return MENU;

            if (keys_pressed & J_A) {
                sfx_hit();
                p_hand[p_count]  = ((uint8_t)rand() % 13) + 1;
                p_suits[p_count] = (uint8_t)rand() & 3;
                draw_card_animated(CARD_X(p_count), 10, p_hand[p_count], p_suits[p_count]);
                p_count++;
                p_score = calc_score(p_hand, p_count);
                draw_score(p_score, 16, 8);
                if (p_score > 21) 
                    state = 2;
                else if (p_score == 21)
                    state = 1;
                draw_text(0, 14, "A:HIT    B:STAND  ");
            }

            if (keys_pressed & J_B) {
                state = 1;
                sfx_stand();
            }

            if ((keys_pressed & J_START) && p_count == 2
                                         && (bet << 1) <= player_bank->money) {
                sfx_double_down();
                doubled_down = 1;
                bet <<= 1;
                draw_text(1, 2, "BET:");
                draw_money(bet, 6, 2);
                transition_flash(2);
                p_hand[p_count]  = ((uint8_t)rand() % 13) + 1;
                p_suits[p_count] = (uint8_t)rand() & 3;
                draw_card_animated(CARD_X(p_count), 10,
                                   p_hand[p_count], p_suits[p_count]);
                p_count++;
                p_score = calc_score(p_hand, p_count);
                draw_score(p_score, 16, 8);
                state = (p_score > 21) ? 2 : 1;
            }
            wait_vbl_done();
        }
        draw_text(0, 14, "                   ");

        if (state == 1 || state == 3) {
            sfx_card_flip();
            delay(200);
            draw_card(4, 5, d_hand[1], d_suits[1]);
            d_score = calc_score(d_hand, d_count);
            draw_text(13, 3, "SC:");
            draw_score(d_score, 16, 3);
            wait_vbl_done();
            delay(600);

            if (state != 3) {
                while (d_score < 17) {
                    d_hand[d_count]  = ((uint8_t)rand() % 13) + 1;
                    d_suits[d_count] = (uint8_t)rand() & 3;
                    draw_card_animated(CARD_X(d_count), 5,
                                       d_hand[d_count], d_suits[d_count]);
                    d_count++;
                    d_score = calc_score(d_hand, d_count);
                    draw_score(d_score, 16, 3);
                    delay(600);
                }
            }
        } else {
            sfx_card_flip();
            draw_card(4, 5, d_hand[1], d_suits[1]);
            d_score = calc_score(d_hand, d_count);
            draw_text(13, 3, "SC:");
            draw_score(d_score, 16, 3);
        }

        {
            uint8_t player_bj = (p_score == 21 && p_count == 2);
            uint8_t dealer_bj = (d_score == 21 && d_count == 2);

            delay(200);

            if (p_score > 21) {
                transition_shake();
                sfx_lose();
                draw_text(1, 14, "BUST! YOU LOSE  ");
                if (player_bank->money >= bet)
                    player_bank->money -= bet;
                else
                    player_bank->money = 0;

            } else if (player_bj && !dealer_bj) {
                transition_flash(4);
                sfx_blackjack();
                player_bank->money += bet + (bet >> 1);
                draw_text(1, 14, "BLACKJACK! 3:2  ");
            } else if (player_bj && dealer_bj) {
                sfx_push();
                draw_text(1, 14, "PUSH! (BJ TIE)  ");
            } else if (d_score > 21) {
                transition_flash(2); sfx_win();
                draw_text(1, 14, "DEALER BUST! WIN");
                player_bank->money += bet;
            } else if (p_score > d_score) {
                transition_flash(2); sfx_win();
                draw_text(1, 14, "YOU WIN!        ");
                player_bank->money += bet;
            } else if (p_score < d_score) {
                transition_shake(); sfx_lose();
                draw_text(1, 14, "DEALER WINS     ");
                if (player_bank->money >= bet)
                    player_bank->money -= bet;
                else 
                    player_bank->money = 0;
            } else {
                sfx_push();
                draw_text(1, 14, "PUSH! (TIE)     ");
            }
        }
        draw_money(player_bank->money, 0, 1);
        draw_text(1, 16, "A:RETRY SEL:QUIT");
        wait_pad_release();

        while (1) {
            keys = joypad();
            if (keys & J_A) {
                sfx_confirm();
                break;
            }
            if (keys & J_SELECT)
                return MENU;
            wait_vbl_done();
        }

        if (player_bank->money == 0) {
            transition_wipe_down();
            sfx_game_over();
            draw_text(2, 8,  "GAME OVER!");
            draw_text(1, 10, "NO MORE FUNDS");
            delay(2500);
            return MENU;
        }
    }
}