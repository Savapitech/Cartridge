#include <gb/gb.h>
#include <rand.h>

#include "../assets/black_jack_asset.h"
#include "../audio/audio.h"
#include "../casino.h"
#include "../utils/transitions.h"
#include "../utils/utils.h"
#include "games.h"
#include "menu.h"
#include "poker.h"

static const char card_chars[] = " A23456789TJQK";

static const char *hand_names[10] = {
    "  NO WIN      ", "JACKS OR BETTER", "  TWO PAIR    ", " THREE OF KIND",
    "   STRAIGHT   ", "    FLUSH     ",  "  FULL HOUSE  ", " FOUR OF KIND ",
    "STRAIGHT FLUSH", " ROYAL FLUSH! ",
};

/* Pay multipliers indexed by hand rank */
static const uint8_t pay_mult[10] = {0, 1, 2, 3, 4, 6, 9, 25, 50, 250};

static void deal_card(uint8_t x, uint8_t y, uint8_t val, uint8_t suit) {
  ch4_play(8, 0, 1, 5, 0, 32);
  load_card_at(x, y, val, suit);
  delay(80);
}

static void draw_hold_row(uint8_t *held) {
  uint8_t i;
  for (i = 0; i < POKER_CARDS; i++)
    draw_text(CPOS(i), ROW_HOLD, held[i] ? "HD" : "  ");
}

static void draw_cursor_row(uint8_t cursor) {
  uint8_t i;
  for (i = 0; i < POKER_CARDS; i++)
    draw_text(CPOS(i), ROW_CURSOR, (i == cursor) ? "^^" : "  ");
}

static uint8_t evaluate_hand(uint8_t *val, uint8_t *suit) {
  uint8_t cnt[14];
  uint8_t scnt[4];
  uint8_t i, pairs, trips, quads;
  uint8_t is_flush, is_str;
  uint8_t high_pair;
  uint8_t mn, mx, uniq;

  for (i = 0; i < 14; i++)
    cnt[i] = 0;
  for (i = 0; i < 4; i++)
    scnt[i] = 0;
  for (i = 0; i < 5; i++) {
    cnt[val[i]]++;
    scnt[suit[i]]++;
  }

  /* Flush */
  is_flush = 0;
  for (i = 0; i < 4; i++)
    if (scnt[i] == 5) {
      is_flush = 1;
      break;
    }

  /* Straight */
  mn = 14;
  mx = 0;
  uniq = 0;
  for (i = 1; i <= 13; i++) {
    if (cnt[i]) {
      uniq++;
      if (i < mn)
        mn = i;
      if (i > mx)
        mx = i;
    }
  }
  is_str = (uniq == 5 && mx - mn == 4);
  if (!is_str && cnt[1] && cnt[2] && cnt[3] && cnt[4] && cnt[5])
    is_str = 1;
  if (!is_str && cnt[1] && cnt[10] && cnt[11] && cnt[12] && cnt[13])
    is_str = 1;

  /* Pairs / trips / quads */
  pairs = trips = quads = high_pair = 0;
  for (i = 1; i <= 13; i++) {
    if (cnt[i] == 2) {
      pairs++;
      if (i == 1 || i >= 11)
        high_pair = 1;
    }
    if (cnt[i] == 3)
      trips++;
    if (cnt[i] == 4)
      quads++;
  }

  if (is_flush && is_str) {
    if (cnt[1] && cnt[10] && cnt[11] && cnt[12] && cnt[13])
      return HAND_ROYAL_FLUSH;
    return HAND_STRAIGHT_FLUSH;
  }
  if (quads)
    return HAND_FOUR_KIND;
  if (trips && pairs)
    return HAND_FULL_HOUSE;
  if (is_flush)
    return HAND_FLUSH;
  if (is_str)
    return HAND_STRAIGHT;
  if (trips)
    return HAND_THREE_KIND;
  if (pairs == 2)
    return HAND_TWO_PAIR;
  if (high_pair)
    return HAND_JACKS_BETTER;
  return HAND_NOTHING;
}

static uint8_t choose_bet(bank_t *player_bank) {
  const uint8_t opts[4] = {5, 10, 25, 50};
  uint8_t selected = 1;
  uint8_t keys = 0, prev_keys, pressed;
  uint8_t i;

  transition_wipe_down();
  CLEAR_BKG;
  draw_text(0, 0, player_bank->name);
  draw_money(player_bank->money, 0, 1);
  draw_text(3, 2, "POKER 5 CARD DRAW");
  draw_text(3, 3, "PLACE YOUR BET");
  draw_text(2, 4, "UP/DOWN:SEL  A:GO");

  while (opts[selected] > player_bank->money && selected > 0)
    selected--;

  draw_text(0, ROW_PAY1, "RF:250 SF:50  4K:25");
  draw_text(0, ROW_PAY2, "FH:9   FL:6   ST:4 ");
  draw_text(0, ROW_PAY3, "3K:3   2P:2   JP:1 ");

  while (1) {
    for (i = 0; i < 4; i++) {
      uint8_t row = 6 + i;
      if (opts[i] > player_bank->money) {
        draw_text(3, row, "-- (NO FUNDS)  ");
      } else {
        draw_text(2, row, (i == selected) ? "> BET:" : "  BET:");
        draw_money(opts[i], 9, row);
      }
    }

    prev_keys = keys;
    keys = joypad();
    pressed = (keys ^ prev_keys) & keys;

    if (pressed & J_SELECT)
      return 0;
    if (pressed & J_UP) {
      int8_t next = (int8_t)selected - 1;
      while (next >= 0 && opts[next] > player_bank->money)
        next--;
      if (next >= 0) {
        selected = (uint8_t)next;
        play_select();
      }
    }
    if (pressed & J_DOWN) {
      uint8_t next = selected + 1;
      while (next < 4 && opts[next] > player_bank->money)
        next++;
      if (next < 4) {
        selected = next;
        play_select();
      }
    }
    if ((pressed & J_A) && opts[selected] <= player_bank->money) {
      play_confirm();
      return opts[selected];
    }
    wait_vbl_done();
  }
}

uint8_t poker(bank_t *player_bank) {
  uint8_t hand_val[5], hand_suit[5];
  uint8_t held[5];
  uint8_t cursor, i, bet, rank;
  uint16_t payout;
  uint8_t keys = 0, prev_keys, pressed;

  audio_init();

  while (1) {
    bet = choose_bet(player_bank);
    if (bet == 0)
      return MENU;

    player_bank->money -= bet;

    transition_wipe_up();
    reset_card_memory();
    CLEAR_BKG;
    draw_text(0, 0, player_bank->name);
    draw_money(player_bank->money, 0, 1);
    draw_text(0, ROW_BET_TXT, "BET:");
    draw_money(bet, 5, ROW_BET_TXT);

    for (i = 0; i < POKER_CARDS; i++) {
      hand_val[i] = ((uint8_t)rand() % 13) + 1;
      hand_suit[i] = (uint8_t)rand() & 3;
      held[i] = 0;
      deal_card(CPOS(i), ROW_CARD_TOP, hand_val[i], hand_suit[i]);
    }

    draw_text(0, ROW_HINT, "A:HOLD  B:DRAW  SEL:Q");

    draw_text(0, ROW_PAY1, "RF:250 SF:50  4K:25 ");
    draw_text(0, ROW_PAY2, "FH:9   FL:6   ST:4  ");
    draw_text(0, ROW_PAY3, "3K:3   2P:2   JP:1  ");

    cursor = 0;
    draw_cursor_row(cursor);
    wait_pad_release();

    while (1) {
      prev_keys = keys;
      keys = joypad();
      pressed = (keys ^ prev_keys) & keys;

      if (pressed & J_SELECT)
        return MENU;

      if ((pressed & J_RIGHT) && cursor < POKER_CARDS - 1) {
        cursor++;
        draw_cursor_row(cursor);
        play_select();
        wait_pad_release();
      }
      if ((pressed & J_LEFT) && cursor > 0) {
        cursor--;
        draw_cursor_row(cursor);
        play_select();
        wait_pad_release();
      }
      if (pressed & J_A) {
        held[cursor] ^= 1;
        draw_hold_row(held);
        play_select();
        wait_pad_release();
      }
      if (pressed & J_B) {
        play_confirm();
        break;
      }
      wait_vbl_done();
    }

    for (i = 0; i < POKER_CARDS; i++) {
      if (!held[i]) {
        hand_val[i] = ((uint8_t)rand() % 13) + 1;
        hand_suit[i] = (uint8_t)rand() & 3;
        deal_card(CPOS(i), ROW_CARD_TOP, hand_val[i], hand_suit[i]);
      }
    }
    draw_text(0, ROW_HOLD, "                    ");
    draw_text(0, ROW_CURSOR, "                    ");

    rank = evaluate_hand(hand_val, hand_suit);
    payout = (uint16_t)bet * pay_mult[rank];

    draw_text(2, ROW_HAND, hand_names[rank]);

    if (payout > 0) {
      player_bank->money += payout;
      draw_money(player_bank->money, 0, 1);
      draw_text(1, ROW_RESULT, "WIN! +");
      draw_money(payout, 8, ROW_RESULT);
      transition_flash(rank >= HAND_FOUR_KIND ? 4 : 2);
      play_win();
    } else {
      draw_money(player_bank->money, 0, 1);
      draw_text(1, ROW_RESULT, "NO WIN...     ");
      transition_shake();
      play_lose();
    }

    draw_text(0, ROW_HINT, "A:RETRY   SEL:QUIT  ");
    wait_pad_release();

    while (1) {
      keys = joypad();
      if (keys & J_A) {
        play_confirm();
        break;
      }
      if (keys & J_SELECT)
        return MENU;
      wait_vbl_done();
    }

    if (player_bank->money == 0) {
      transition_wipe_down();
      play_game_over();
      draw_text(2, 8, "GAME OVER!");
      draw_text(1, 10, "NO MORE FUNDS");
      delay(2500);
      return MENU;
    }
  }
}
