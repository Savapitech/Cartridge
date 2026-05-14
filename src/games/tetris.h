#ifndef TETRIS_H
#define TETRIS_H

#include "../casino.h"

#define BOARD_W 10
#define BOARD_H 16
#define SCR_BOARD_X 1
#define SCR_BOARD_Y 1
#define T_BLOCK 112
#define T_PIECES 7
#define DAS_INIT 16
#define DAS_REP 6

uint8_t tetris(bank_t *player_bank);

#endif
