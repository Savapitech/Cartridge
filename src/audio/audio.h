#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>

#define NOTE_C4_LO 10
#define NOTE_C4_HI 6
#define NOTE_D4_LO 27
#define NOTE_D4_HI 6
#define NOTE_E4_LO 44
#define NOTE_E4_HI 6
#define NOTE_F4_LO 53
#define NOTE_F4_HI 6
#define NOTE_G4_LO 68
#define NOTE_G4_HI 6
#define NOTE_A4_LO 80
#define NOTE_A4_HI 6
#define NOTE_B4_LO 91
#define NOTE_B4_HI 6
#define NOTE_C5_LO 97
#define NOTE_C5_HI 6
#define NOTE_D5_LO 105
#define NOTE_D5_HI 6
#define NOTE_E5_LO 112
#define NOTE_E5_HI 6

void audio_init(void);

void ch1_play(uint8_t freq_lo, uint8_t freq_hi, uint8_t duty, uint8_t len,
              uint8_t env_start, uint8_t env_dir, uint8_t env_step);
void ch2_play(uint8_t freq_lo, uint8_t freq_hi, uint8_t duty, uint8_t len,
              uint8_t env_start, uint8_t env_dir, uint8_t env_step);
void ch4_play(uint8_t env_start, uint8_t env_dir, uint8_t env_step,
              uint8_t shift, uint8_t width, uint8_t len);

void play_select(void);
void play_confirm(void);
void play_win(void);
void play_lose(void);
void play_game_over(void);

#endif
