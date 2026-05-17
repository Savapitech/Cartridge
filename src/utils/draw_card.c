#include "../assets/card.h"


const uint8_t* const CARD_TILES[4][13] = {
    {
        c_A_coeur_tiles,  c_2_coeur_tiles,  c_3_coeur_tiles,  c_4_coeur_tiles, 
        c_5_coeur_tiles,  c_6_coeur_tiles,  c_7_coeur_tiles,  c_8_coeur_tiles, 
        c_9_coeur_tiles,  c_10_coeur_tiles, c_J_coeur_tiles,  c_Q_coeur_tiles, 
        c_K_coeur_tiles
    },
    {
        c_A_pique_tiles,  c_2_pique_tiles,  c_3_pique_tiles,  c_4_pique_tiles, 
        c_5_pique_tiles,  c_6_pique_tiles,  c_7_pique_tiles,  c_8_pique_tiles, 
        c_9_pique_tiles,  c_10_pique_tiles, c_J_pique_tiles,  c_Q_pique_tiles, 
        c_K_pique_tiles
    },
    {
        c_A_carreau_tiles,  c_2_carreau_tiles,  c_3_carreau_tiles,  c_4_carreau_tiles, 
        c_5_carreau_tiles,  c_6_carreau_tiles,  c_7_carreau_tiles,  c_8_carreau_tiles, 
        c_9_carreau_tiles,  c_10_carreau_tiles, c_J_carreau_tiles,  c_Q_carreau_tiles, 
        c_K_carreau_tiles
    },
    {
        c_A_trefle_tiles,  c_2_trefle_tiles,  c_3_trefle_tiles,  c_4_trefle_tiles, 
        c_5_trefle_tiles,  c_6_trefle_tiles,  c_7_trefle_tiles,  c_8_trefle_tiles, 
        c_9_trefle_tiles,  c_10_trefle_tiles, c_J_trefle_tiles,  c_Q_trefle_tiles, 
        c_K_trefle_tiles
    }
};

#define CARD_WIDTH 2
#define CARD_HEIGHT 2
#define TILES_PER_CARD 4 

#define CARD_START_TILE 128

uint8_t loaded_cards[7];
uint8_t next_free_tile = CARD_START_TILE;

void reset_card_memory(void) {
    uint8_t i;
    for (i = 0; i < 7; i++) {
        loaded_cards[i] = 0;
    }
    next_free_tile = CARD_START_TILE;
}

void load_card_at(uint8_t x, uint8_t y, uint8_t val, uint8_t suit) {
    uint8_t card_id = (suit * 13) + (val - 1);
    uint8_t byte_idx = card_id >> 3;
    uint8_t bit_idx = card_id & 7;
    uint8_t tile_id;
    uint8_t i;

    if ((loaded_cards[byte_idx] & (1 << bit_idx)) == 0) {
        tile_id = next_free_tile;
        set_bkg_data(tile_id, TILES_PER_CARD, CARD_TILES[suit][val - 1]);
        loaded_cards[byte_idx] |= (1 << bit_idx);
        next_free_tile += TILES_PER_CARD;
    } else {
        uint8_t count = 0;
        for (i = 0; i < card_id; i++) {
            if (loaded_cards[i >> 3] & (1 << (i & 7))) {
                count++;
            }
        }
        tile_id = CARD_START_TILE + (count * TILES_PER_CARD);
    }
    uint8_t card_map[4]; 
    for(i = 0; i < 4; i++) {
        card_map[i] = tile_id + i;
    }
    set_bkg_tiles(x, y, CARD_WIDTH, CARD_HEIGHT, card_map);
}