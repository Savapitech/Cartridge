#ifndef CASINO_ASSET_H
#define CASINO_ASSET_H

#include <stdint.h>

#define T_FELT 96
#define T_BRD_TL 97
#define T_BRD_TR 98
#define T_BRD_BL 99
#define T_BRD_BR 100
#define T_BRD_H 101
#define T_BRD_HB 102
#define T_BRD_VL 103
#define T_BRD_VR 104
#define T_DIAMOND 105
#define T_CHIP 106
#define T_BRD_DIV 107

extern const uint8_t casino_felt[16];
extern const uint8_t casino_brd_tl[16];
extern const uint8_t casino_brd_tr[16];
extern const uint8_t casino_brd_bl[16];
extern const uint8_t casino_brd_br[16];
extern const uint8_t casino_brd_h[16];
extern const uint8_t casino_brd_hb[16];
extern const uint8_t casino_brd_vl[16];
extern const uint8_t casino_brd_vr[16];
extern const uint8_t casino_diamond[16];
extern const uint8_t casino_chip[16];
extern const uint8_t casino_brd_div[16];

void load_casino_tiles(void);
void draw_casino_bg(void);
void draw_casino_hdiv(uint8_t row);

#endif
