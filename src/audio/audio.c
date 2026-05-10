#include <gb/gb.h>

#include "audio.h"

void audio_init(void) {
  NR52_REG = 0x80;
  NR50_REG = 0x77;
  NR51_REG = 0xFF;
}

void ch1_play(uint8_t freq_lo, uint8_t freq_hi, uint8_t duty, uint8_t len,
              uint8_t env_start, uint8_t env_dir, uint8_t env_step) {
  NR11_REG = (duty << 6) | (64 - len);
  NR12_REG = (env_start << 4) | (env_dir << 3) | env_step;
  NR13_REG = freq_lo;
  NR14_REG = 0x80 | (len ? 0x40 : 0x00) | (freq_hi & 0x07);
}

void ch2_play(uint8_t freq_lo, uint8_t freq_hi, uint8_t duty, uint8_t len,
              uint8_t env_start, uint8_t env_dir, uint8_t env_step) {
  NR21_REG = (duty << 6) | (64 - len);
  NR22_REG = (env_start << 4) | (env_dir << 3) | env_step;
  NR23_REG = freq_lo;
  NR24_REG = 0x80 | (len ? 0x40 : 0x00) | (freq_hi & 0x07);
}

void ch4_play(uint8_t env_start, uint8_t env_dir, uint8_t env_step,
              uint8_t shift, uint8_t width, uint8_t len) {
  NR41_REG = 64 - len;
  NR42_REG = (env_start << 4) | (env_dir << 3) | env_step;
  NR43_REG = (shift << 4) | (width << 3);
  NR44_REG = 0x80 | (len ? 0x40 : 0x00);
}

void play_select(void) { ch2_play(NOTE_E4_LO, NOTE_E4_HI, 2, 6, 8, 0, 5); }

void play_confirm(void) {
  NR21_REG = 0x80;
  NR22_REG = 0xA2;
  NR23_REG = 0x70;
  NR24_REG = 0xC4;
  delay(60);
  NR21_REG = 0x80;
  NR22_REG = 0xA1;
  NR23_REG = 0x90;
  NR24_REG = 0xC4;
  delay(80);
}

void play_win(void) {
  ch1_play(NOTE_C4_LO, NOTE_C4_HI, 2, 12, 14, 0, 2);
  delay(120);
  ch1_play(NOTE_E4_LO, NOTE_E4_HI, 2, 12, 14, 0, 2);
  delay(120);
  ch1_play(NOTE_G4_LO, NOTE_G4_HI, 2, 12, 14, 0, 2);
  delay(120);
  ch1_play(NOTE_C5_LO, NOTE_C5_HI, 2, 20, 15, 0, 1);
}

void play_lose(void) {
  ch1_play(NOTE_C4_LO, NOTE_C4_HI, 1, 12, 12, 0, 4);
  delay(100);
  ch1_play(NOTE_B4_LO, NOTE_B4_HI - 1, 1, 12, 10, 0, 4);
  delay(100);
  ch4_play(12, 0, 2, 3, 0, 30);
  delay(200);
  ch4_play(14, 1, 1, 3, 0, 40);
}

void play_game_over(void) {
  ch1_play(NOTE_C5_LO, NOTE_C5_HI, 0, 30, 14, 0, 1);
  delay(250);
  ch1_play(NOTE_B4_LO, NOTE_B4_HI, 0, 30, 12, 0, 2);
  delay(250);
  ch1_play(NOTE_A4_LO, NOTE_A4_HI, 0, 30, 10, 0, 2);
  delay(250);
  ch1_play(NOTE_G4_LO, NOTE_G4_HI, 0, 40, 8, 0, 3);
  ch4_play(12, 0, 2, 4, 0, 40);
  delay(400);
  ch4_play(14, 1, 1, 5, 0, 40);
}
