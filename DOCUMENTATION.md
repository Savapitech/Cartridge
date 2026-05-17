# Casino GB — Technical Documentation

> Complete technical reference for the Cartridge project: a Game Boy cartridge containing a casino game collection, written in C using the GBDK (Game Boy Development Kit). This document covers the full project architecture and provides a step-by-step guide for implementing a new game.

---

## Table of Contents

### Part I — Architecture
1. [Project Overview](#1-project-overview)
2. [Development Environment](#2-development-environment)
3. [Directory Structure](#3-directory-structure)
4. [Build System](#4-build-system)
5. [Game Boy Hardware Constraints](#5-game-boy-hardware-constraints)
6. [Core Types and Structures](#6-core-types-and-structures)
7. [Entry Point — main.c](#7-entry-point--mainc)
8. [Game Dispatcher — games.c / games.h](#8-game-dispatcher--gamesc--gamesh)
9. [Save System](#9-save-system)
10. [Audio System](#10-audio-system)
11. [Graphics and Asset System](#11-graphics-and-asset-system)
12. [Utilities](#12-utilities)
13. [Transitions and Visual Effects](#13-transitions-and-visual-effects)
14. [Hardware Initialization](#14-hardware-initialization)
15. [Game Catalogue](#15-game-catalogue)
16. [Full Execution Flow](#16-full-execution-flow)
17. [VRAM Memory Map](#17-vram-memory-map)
18. [Code Conventions](#18-code-conventions)

### Part II — Implementing a New Game
19. [Overview of the Integration Process](#19-overview-of-the-integration-process)
20. [Step 1 — Create the Game Header](#20-step-1--create-the-game-header)
21. [Step 2 — Create the Game Source File](#21-step-2--create-the-game-source-file)
22. [Step 3 — Register the Game in the Dispatcher](#22-step-3--register-the-game-in-the-dispatcher)
23. [Step 4 — Build and Test](#23-step-4--build-and-test)
24. [Input Handling Patterns](#24-input-handling-patterns)
25. [Drawing to Screen](#25-drawing-to-screen)
26. [Using Audio](#26-using-audio)
27. [Handling Player Money](#27-handling-player-money)
28. [Adding Custom Assets](#28-adding-custom-assets)
29. [Complete Annotated Example — Coin Flip Game](#29-complete-annotated-example--coin-flip-game)
30. [Common Pitfalls](#30-common-pitfalls)

---

# Part I — Architecture

---

## 1. Project Overview

**Casino GB** is an original Game Boy cartridge containing multiple casino-style games. The entire project is written in C and compiled into a `.gb` ROM executable on Game Boy DMG (original), Game Boy Color, and any compatible emulator.

### Global characteristics

|-----------|-------|
| Language | C (GBDK / SDCC) |
| Target | Game Boy DMG / GBC |
| Compiler | `lcc` (GBDK wrapper around SDCC) |
| Generated ROM | `a.gb` + `a.ihx` |
| Number of games | 5 |
| Save slots | 8 (battery-backed RAM) |

### Available menu

| Index | Constant | Display name | Type |
|-------|----------|--------------|------|
| 0 | `MENU` | Menu | Navigation |
| 1 | `SLOT` | Slots | Slot machine |
| 2 | `BLAC_JACK` | Black Jack | Card game |
| 3 | `ROULETTE` | Roulette | Table game |
| 4 | `POKER` | Poker | Card game |
| 5 | `TAX_MONEY` | Tax money | Easter egg |
| 6 | `CHEAT_CODE` | Player name | Profile / cheat |
| 7 | `LEADERBOARD` | Scores | High scores |

---

## 2. Development Environment

### Nix Flake

The project uses `flake.nix` to reproduce the development environment deterministically. The `direnv` configuration (`.envrc`) automatically activates the environment when entering the directory.

```
flake.nix      → Nix shell definition
.envrc         → use flake (automatic direnv activation)
gbdk/          → GBDK bundled in the repo (lcc, png2asset, etc.)
```

Nix packages provided:
- `retroarch` — emulator for testing the ROM
- `clang-tools` — for `clang-format`

The `lcc` binary is added to `PATH` via `gbdk/bin/`. On the reference machine `pluton`, the path is resolved dynamically in the Makefile:

```makefile
ifeq ($(shell hostname),pluton)
GBDK_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))gbdk
CC = $(GBDK_DIR)/bin/lcc
else
CC = lcc
endif
```

### Prerequisites

```bash
# Activate the environment
direnv allow     # or: nix develop

# Verify lcc is available
which lcc

# Compile
make

# Test
make launch      # opens RetroArch with gambatte_libretro
```

---

## 3. Directory Structure

```
cartridge/
├-- Makefile                    ← Main build file
├-- utils.mk                    ← Makefile helpers (colors, logs)
├-- flake.nix                   ← Reproducible Nix environment
├-- .envrc                      ← direnv: activates Nix shell
├-- a.gb                        ← Compiled Game Boy ROM
├-- a.ihx                       ← Intel HEX format (intermediate)
│
├-- gbdk/                       ← GBDK toolchain
│   └-- bin/lcc                 ← C → Game Boy compiler
│
├-- asset/                      ← Raw PNG sources (not compiled)
│   ├-- background/
│   │   └-- slotMachine.png
│   ├-- CasinoCardAsset.png
│   └-- Sonic.png
│
└-- src/                        ← C source code
    ├-- main.c                  ← Entry point
    ├-- casino.h                ← Core types (bank_t, macros)
    │
    ├-- assets/                 ← Compiled tile/sprite data (png2asset)
    │   ├-- card/               ← 52 cards × 2 files (.c + .h)
    │   │   ├-- c_A_coeur.c/.h
    │   │   ├-- c_2_coeur.c/.h
    │   │   └-- ... (104 files total)
    │   ├-- black_jack_asset.c/.h    ← Suit symbols
    │   ├-- roulette_asset.c/.h      ← Wheel / felt tiles
    │   ├-- slot_asset.c/.h          ← Slot machine symbols
    │   ├-- menu_banner.c/.h         ← Animated banner light sprites
    │   ├-- load_screen.c/.h         ← Splash screen
    │   ├-- casino_asset.c/.h        ← Casino UI tiles (borders, chips)
    │   ├-- asset_croupier1.c/.h     ← Dealer sprite (Black Jack)
    │   ├-- LeftLeftEye.c/.h         ← Dealer eye animation
    │   ├-- LeftRightEye.c/.h
    │   └-- card.h                   ← Master include for all 52 cards
    │
    ├-- audio/                  ← Hardware sound synthesis
    │   ├-- audio.c
    │   └-- audio.h
    │
    ├-- games/                  ← Game implementations
    │   ├-- games.c             ← Dispatch table (game_tab[])
    │   ├-- games.h             ← Declarations + index constants
    │   ├-- menu.c/.h           ← Main menu + tax_money
    │   ├-- black_jack.c/.h     ← Black Jack
    │   ├-- slot.c/.h           ← Slot machine
    │   ├-- roulette.c/.h       ← Roulette
    │   ├-- poker.c/.h          ← 5-card draw poker
    │   ├-- tetris.c/.h         ← Tetris
    │   ├-- leaderboard.c/.h    ← High score screen
    │   └-- cheat.c             ← Profile / cheat code handler
    │
    ├-- init/                   ← Game Boy hardware initialization
    │   ├-- init_game_boy.c     ← LCD, PPU, sprite mode registers
    │   ├-- init_font.c         ← Font loading into VRAM
    │   └-- init.h
    │
    ├-- save/                   ← Battery RAM persistence
    │   ├-- save.c
    │   └-- save.h
    │
    └-- utils/                  ← Shared utility functions
        ├-- draw_text.c         ← ASCII text rendering
        ├-- draw_int.c          ← Money / integer rendering
        ├-- draw_card.c         ← Dynamic card VRAM cache
        ├-- get_input.c         ← Player name input (D-pad keyboard)
        ├-- transitions.c       ← Screen effects (wipe, flash, shake)
        ├-- wait_pad_release.c  ← Joypad debounce
        ├-- utils.h
        ├-- get_input.h
        └-- transitions.h
```

---

## 4. Build System

### Makefile

The build is managed by GNU Make using the GBDK compiler `lcc`. All sources are discovered via wildcard patterns:

```makefile
SRC = $(wildcard src/*.c)
SRC += $(wildcard src/assets/*.c)
SRC += $(wildcard src/assets/card/*.c)
SRC += $(wildcard src/audio/*.c)
SRC += $(wildcard src/utils/*.c)
SRC += $(wildcard src/games/*.c)
SRC += $(wildcard src/init/*.c)
SRC += $(wildcard src/save/*.c)
```

Each `.c` file is compiled to a `.o` object in `.build/<profile>/` via the `mk-profile` macro. The build is parallelized with `-j`.

### Compilation flags

```makefile
CFLAGS  = -Wf--max-allocs-per-node50000    # SDCC: increase allocations per node
LDFLAGS = -Wm-yt27                          # Cartridge type: MBC5 + RAM + BATTERY
          -Wm-ya1                           # 1 external RAM bank (8 KiB)
```

- `-Wm-yt27`: writes cartridge type `0x1B` (MBC5+RAM+BATTERY) into the ROM header
- `-Wm-ya1`: declares 1 external RAM bank, enabling battery-backed save slots

### Build profiles

| Command | Description | Extra flag |
|---------|-------------|------------|
| `make` | Standard build | — |
| `make fast` | Speed-optimized | `-Wf--opt-code-speed` |
| `make small` | Size-optimized | `-Wf--opt-code-size` |
| `make debug` | Debug mode | `-D DEBUG_MODE` |
| `make re` | Clean rebuild | — |
| `make format` | Format all .c/.h | `clang-format -i` |
| `make check_format` | Verify format | `clang-format --dry-run` |
| `make launch` | Compile + open RetroArch | — |

### Compilation pipeline

```
src/*.c  --┐
           ├--► lcc -c → .build/release/*.o --► lcc (link) → a.ihx → a.gb
src/**/*.c ┘
```

`lcc` is a wrapper around SDCC (Small Device C Compiler) adapted for the Game Boy target.

---

## 5. Game Boy Hardware Constraints

Understanding the hardware constraints is essential for Game Boy development.

### CPU and memory

| Resource | Capacity |
|----------|----------|
| CPU | Sharp LR35902, 4.19 MHz |
| Internal RAM | 8 KiB (0xC000–0xDFFF) |
| VRAM | 8 KiB (0x8000–0x9FFF) |
| External RAM (battery) | 8 KiB (0xA000–0xBFFF) |
| ROM | Max 32 KiB no-bank (MBC: up to 8 MiB) |

### Graphics system (PPU)

The Game Boy uses an **8×8 tile** system. Each pixel is encoded on 2 bits (4 shades of gray).

**VRAM** layout (0x8000–0x9FFF):
- **Tile Data** (0x8000–0x97FF): up to 384 tiles, 16 bytes each
- **BKG Tile Map** (0x9800–0x9BFF): 32×32 tile background map
- **Window Tile Map** (0x9C00–0x9FFF): 32×32 window overlay map

**Screen**: 160×144 pixels = 20×18 visible tiles

**Sprites**: 40 OAM sprites, each 8×8 or 8×16 pixels, 4 attributes (X, Y, tile index, flags)

### Joypad

`P1` register (0xFF00). Accessible through GBDK's `joypad()` function:

```c
J_RIGHT   J_LEFT    J_UP      J_DOWN
J_A       J_B       J_SELECT  J_START
```

### VBL synchronization

The Game Boy redraws the screen 59.73 times per second. VRAM writes must be synchronized with the Vertical Blank period:

```c
wait_vbl_done();   // Wait for end of screen draw before next frame
```

Any VRAM access during rendering (PPU modes 2/3) causes graphical glitches. Always update VRAM either during VBL or with the display off.

### Stack and RAM budget

The Game Boy has only 8 KiB of RAM. SDCC uses a portion for the call stack. Key consequences:
- Avoid large stack-allocated arrays (use `static` or global for big buffers)
- No dynamic memory allocation (`malloc` does not exist in this environment)
- All data structures must have a fixed, known size at compile time

---

## 6. Core Types and Structures

### `casino.h` — Global types

This is the most-included header in the project. It defines the central player type and two screen-clearing macros.

```c
// src/casino.h

typedef struct bank_s {
  char     name[16];    // Player name (max 15 chars + '\0')
  uint8_t  name_size;   // Actual length of the name
  uint32_t money;       // Player balance in game units
} bank_t;

// Clear the entire window layer (fill with tile 0)
#define CLEAR_WIN fill_win_rect(0, 0, 20, 32, 0)

// Clear the entire background layer (fill with tile 0)
#define CLEAR_BKG fill_bkg_rect(0, 0, 32, 32, 0)
```

`bank_t` is passed **by pointer** to every game function. Any modification to `money` or `name` is automatically persisted after the game returns, via `save_write()` in the main loop.

### `game_t` — Entry in the dispatch table

```c
// src/games/games.h

typedef struct game_s {
  char     name[16];          // Display name shown in the menu
  uint8_t  name_size;         // Length of the name
  uint8_t (*game)(bank_t *);  // Function pointer to the game
} game_t;
```

Every game is a **function pointer** of type `uint8_t (*)(bank_t *)` that:
- Receives the player context as input
- Returns the **index of the next game** to launch

### Game index constants

```c
// src/games/games.h

#define MENU         0
#define SLOT         MENU + 1        // 1
#define BLAC_JACK    SLOT + 1        // 2
#define ROULETTE     BLAC_JACK + 1   // 3
#define POKER        ROULETTE + 1    // 4
#define TAX_MONEY    POKER + 1       // 5
#define CHEAT_CODE   TAX_MONEY + 1   // 6
#define LEADERBOARD  CHEAT_CODE + 1  // 7
```

These constants are defined incrementally: inserting a new game between two existing ones only requires reordering the defines, not renumbering manually. They map exactly to the order of entries in `game_tab[]`.

---

## 7. Entry Point — `main.c`

```c
// src/main.c

void main(void) {
  bank_t  bank     = {.name = "Player", .name_size = 6, .money = 0};
  uint8_t game_idx = 0;

  // 1. Standard gray palette
  BGP_REG = 0xE4U;

  // 2. Load splash screen with wave effect
  set_bkg_data(0, load_screen_TILE_COUNT, load_screen_tiles);
  set_bkg_tiles(0, 0, 20, 18, load_screen_map);
  SHOW_BKG;
  DISPLAY_ON;
  draw_wave();

  // 3. Wait for START button
  while (!(joypad() & J_START)) {
    delay_counter++;
    if (delay_counter >= 4) { wave_counter++; delay_counter = 0; }
    wait_vbl_done();
  }

  // 4. Tear down LCD interrupt used for the wave effect
  disable_interrupts();
  remove_LCD(interupt_lcd);
  SCX_REG = 0;
  set_interrupts(VBL_IFLAG);
  enable_interrupts();

  // 5. Hardware and font initialization
  init();
  init_font();

  // 6. Load existing save or use defaults
  save_load(bank.name, &bank);
  draw_text(0, 0, bank.name);

  // 7. Infinite main loop
  for (;;) {
    game_idx = game_tab[game_idx].game(&bank);  // Run current game
    save_write(&bank);                           // Auto-save after every game
    wait_vbl_done();
  }
}
```

### Wave effect (splash screen)

The wave effect is implemented via an **LCD STAT interrupt** (HBlank mode). At each scanline, the `SCX_REG` (horizontal scroll) register is modified using a pre-computed sine table:

```c
const uint8_t wave_sinus[32] = {
    0, 1, 2, 3, 4, 4, 3, 2, 1, 0, 255, 254, 253, 252, 252, 253,
    254, 255, 0, 1, 2, 3, 4, 4, 3, 2, 1, 0, 255, 254, 253, 252
};

void interupt_lcd(void) {
  SCX_REG = wave_sinus[(LY_REG + wave_counter) & 31];
}
```

- `LY_REG`: current scanline number (0–143)
- `wave_counter`: phase offset, incremented every 4 frames
- `& 31`: wraps into the 32-entry sine table

---

## 8. Game Dispatcher — `games.c` / `games.h`

### The `game_tab[]` table

```c
// src/games/games.c

#define GAME_NAME(a) a, sizeof(a)

const game_t game_tab[] = {
  {GAME_NAME("Menu"),        menu},
  {GAME_NAME("Slots"),       slot_machine},
  {GAME_NAME("Black Jack"),  black_jack},
  {GAME_NAME("Roulette"),    roulette},
  {GAME_NAME("Poker"),       poker},
  {GAME_NAME("Tax money"),   tax_money},
  {GAME_NAME("Player name"), cheat_code},
  {GAME_NAME("Scores"),      leaderboard},
};

const uint8_t GAME_TAB_SIZE = 8;
```

The `GAME_NAME(a)` macro expands to `a, sizeof(a)`, initializing both the `name[16]` and `name_size` fields of `game_t`. `sizeof("Menu")` includes the null terminator, so `name_size = 5` for "Menu".

### Navigation mechanism

The main loop in `main()` is trivially simple:

```c
game_idx = game_tab[game_idx].game(&bank);
```

Each game returns the index of the **next game**. To return to the menu, a game returns `MENU` (0). To chain directly into another game without passing through the menu, it returns that game's index constant.

The menu itself reads the `GAME_TAB_SIZE` constant to know how many entries to cycle through, and returns whichever index the player selected.

---

## 9. Save System

### External RAM address space

Battery RAM occupies **0xA000–0xBFFF** (8 KiB). It is enabled and disabled via the GBDK macros `ENABLE_RAM` / `DISABLE_RAM`, which write to MBC5 control registers.

### Save slot format

```c
// src/save/save.h

#define SAVE_MAGIC  0xCA   // Validity marker
#define MAX_SAVES   8      // Number of save slots
#define SLOT_BYTES  16     // Size of one slot in bytes
#define SAVE_NONE   0xFF   // "Not found" sentinel value

typedef struct {
  uint8_t  magic;     // 0xCA if slot is valid
  char     name[11];  // Player name (10 chars + '\0')
  uint32_t money;     // Balance (4 bytes)
} save_slot_t;        // Total: 1 + 11 + 4 = 16 bytes
```

Memory layout:

```
0xA000  [slot 0: magic(1) | name(11) | money(4)]  ← 16 bytes
0xA010  [slot 1: magic(1) | name(11) | money(4)]
0xA020  [slot 2: ...]
...
0xA070  [slot 7: magic(1) | name(11) | money(4)]
0xA080  (unused)
```

### Public API

```c
// Find a slot by player name — returns index or SAVE_NONE
uint8_t save_find(const char *name);

// Load a slot into bank_t — returns 1 if found, 0 otherwise
uint8_t save_load(const char *name, bank_t *bank);

// Write/update the player's slot (creates one if none exists)
void save_write(const bank_t *bank);

// Raw read of a slot — returns 1 if slot is valid (magic == 0xCA)
uint8_t save_read_slot(uint8_t slot, save_slot_t *out);
```

### Low-level access

```c
static volatile uint8_t *slot_addr(uint8_t slot) {
  return (volatile uint8_t *)(0xA000u + (uint16_t)slot * SLOT_BYTES);
}
```

External RAM is accessed via **direct pointer arithmetic** into the address space. The `volatile` keyword prevents the compiler from optimizing away these hardware memory accesses.

### Allocation strategy

1. `save_write()` first searches for an existing slot with `find_slot(name)`
2. If not found, finds the first free slot with `first_free()`
3. A free slot is identified by `magic != SAVE_MAGIC`
4. If all 8 slots are occupied and none match the player name, the write silently fails

### Automatic save in main()

```c
// Load at startup (falls back to defaults if no save found)
save_load(bank.name, &bank);

// Auto-save after every game exit
for (;;) {
  game_idx = game_tab[game_idx].game(&bank);
  save_write(&bank);   // ← always called
  wait_vbl_done();
}
```

---

## 10. Audio System

### Game Boy audio hardware

The Game Boy has 4 audio channels:
- **CH1**: Square wave with frequency sweep
- **CH2**: Square wave (no sweep)
- **CH3**: Arbitrary waveform (32 × 4-bit samples)
- **CH4**: White noise (LFSR-based)

Only CH1, CH2, and CH4 are used in this project.

### Available musical notes

```c
// src/audio/audio.h — Frequencies as (lo, hi) byte pairs
#define NOTE_C4_LO 10   #define NOTE_C4_HI 6    // Middle C
#define NOTE_D4_LO 27   #define NOTE_D4_HI 6
#define NOTE_E4_LO 44   #define NOTE_E4_HI 6
#define NOTE_F4_LO 53   #define NOTE_F4_HI 6
#define NOTE_G4_LO 68   #define NOTE_G4_HI 6
#define NOTE_A4_LO 80   #define NOTE_A4_HI 6
#define NOTE_B4_LO 91   #define NOTE_B4_HI 6
#define NOTE_C5_LO 97   #define NOTE_C5_HI 6
#define NOTE_D5_LO 105  #define NOTE_D5_HI 6
#define NOTE_E5_LO 112  #define NOTE_E5_HI 6
```

The actual frequency in Hz is: `f = 131072 / (2048 - x)` where `x = (freq_hi << 8) | freq_lo`.

### Programming interface

```c
// Silence all channels (call at start of each game)
void audio_init(void);

// Channel 1 — Square wave with envelope
void ch1_play(
  uint8_t freq_lo,    // Frequency low byte
  uint8_t freq_hi,    // Frequency high byte
  uint8_t duty,       // Duty cycle (0–3 → 12.5 / 25 / 50 / 75 %)
  uint8_t len,        // Sound length (0 = continuous)
  uint8_t env_start,  // Initial volume (0–15)
  uint8_t env_dir,    // Envelope direction (0 = decrease, 1 = increase)
  uint8_t env_step    // Volume change speed
);

// Channel 2 — Same interface, no sweep
void ch2_play(freq_lo, freq_hi, duty, len, env_start, env_dir, env_step);

// Channel 4 — White noise
void ch4_play(
  uint8_t env_start,  // Initial volume
  uint8_t env_dir,    // Envelope direction
  uint8_t env_step,   // Speed
  uint8_t shift,      // Noise frequency (4 bits)
  uint8_t width,      // LFSR width (0 = 15-bit, 1 = 7-bit)
  uint8_t len         // Sound length
);
```

### Preset sounds

| Function | Used for |
|----------|----------|
| `play_select()` | Menu cursor navigation |
| `play_confirm()` | Confirming a selection |
| `play_win()` | Simple win |
| `play_lose()` | Loss |
| `play_game_over()` | Game over (no funds left) |

Example from Black Jack:

```c
// Card flip — short noise burst
static void play_card_flip(void) { ch4_play(8, 0, 1, 5, 0, 32); }

// Two-channel win chord with arpeggio
static void play_blackjack(void) {
  ch1_play(NOTE_C5_LO, NOTE_C5_HI, 2, 15, 15, 0, 1);
  ch2_play(NOTE_E5_LO, NOTE_E5_HI, 2, 15, 13, 0, 1);
  delay(150);
  ch1_play(NOTE_E5_LO, NOTE_E5_HI, 2, 15, 15, 0, 1);
  ch2_play(NOTE_G4_LO, NOTE_G4_HI, 2, 15, 13, 0, 1);
  // ...
}
```

---

## 11. Graphics and Asset System

### Asset pipeline

```
PNG source (asset/)
    │
    ▼
png2asset (GBDK tool)
    │
    ▼
.c + .h files generated (src/assets/)
    │
    ▼
Compiled into the project
    │
    ▼
Loaded at runtime into VRAM via set_bkg_data()
```

### Structure of a generated asset file

Example for a playing card:

```c
// src/assets/card/c_A_coeur.h
#define c_A_coeur_TILE_COUNT 4            // 4 tiles of 8×8
extern const uint8_t c_A_coeur_tiles[64]; // 4 × 16 bytes

// src/assets/card/c_A_coeur.c
const uint8_t c_A_coeur_tiles[64] = {
  // Binary data encoded in 2bpp (2 bits per pixel)
  0x00, 0x00, 0x7E, 0x7E, ...
};
```

### Loading tiles into VRAM

```c
// Load tile_count tiles starting at index tile_base in VRAM
set_bkg_data(uint8_t tile_base, uint8_t tile_count, const uint8_t *tiles);

// Place a single tile at (x, y) in the tile map
set_bkg_tile_xy(uint8_t x, uint8_t y, uint8_t tile_id);

// Place a rectangle of tiles
set_bkg_tiles(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t *map);
```

### Dynamic card cache system

VRAM holds a maximum of **384 tiles** total. With 52 cards × 4 tiles = 208 tiles, it is impossible to load all cards simultaneously while also keeping the font and UI tiles.

The `draw_card.c` module implements a simplified **on-demand cache**:

```c
// Internal: 52-bit bitmask (7 bytes) — bit set = card is in VRAM
static uint8_t loaded_cards[7];

// Load a card at position (x, y) — manages the VRAM cache automatically
void load_card_at(uint8_t x, uint8_t y, uint8_t val, uint8_t suit);

// Reset the cache (call between rounds/games)
void reset_card_memory(void);
```

A card's linear index is: `index = suit * 13 + (val - 1)`.

Card values: 1 (Ace) through 13 (King). Suits: 0=hearts, 1=spades, 2=diamonds, 3=clubs.

### Casino UI tiles

```c
// src/assets/casino_asset.h — Tile indices
#define T_FELT      96   // Felt texture
#define T_BRD_TL    97   // Border corner top-left
#define T_BRD_TR    98   // Border corner top-right
#define T_BRD_BL    99   // Border corner bottom-left
#define T_BRD_BR   100   // Border corner bottom-right
#define T_BRD_H    101   // Horizontal border
#define T_BRD_V    102   // Vertical border
#define T_DIAMOND  103   // Decorative diamond
#define T_CHIP     104   // Casino chip

void load_casino_tiles(void);          // Load all casino tiles into VRAM
void draw_casino_bg(void);             // Draw the full casino background
void draw_casino_hdiv(uint8_t row);    // Draw a horizontal divider line
```

### VRAM tile index conventions

| Range | Usage |
|-------|-------|
| 0–95 | Font (ASCII characters 32–127) |
| 96–127 | Casino/game-specific UI tiles |
| 128–131 | Suit symbols (Black Jack) |
| 132–179 | Current game assets (variable) |
| 180–199 | Dealer sprite + eyes (Black Jack) |
| 200–211 | Slot machine symbols |
| 204 | Roulette ball tile |
| 212–383 | Card cache (up to ~42 cards) |

---

## 12. Utilities

### `draw_text()` — Text rendering

```c
// src/utils/draw_text.c
void draw_text(uint8_t x, uint8_t y, const char *str);
```

Iterates through the string character by character. Each character is converted to a tile index via `c - 32` (the font starts at ASCII space 32). Text is drawn in the background layer via `set_bkg_tile_xy()`.

### `draw_money()` — Balance display

```c
// src/utils/draw_int.c
void draw_money(uint32_t money, uint8_t x, uint8_t y);
void render_money(bank_t *player_bank);
```

`draw_money()` converts a `uint32_t` to digit tiles with leading-space suppression (no leading zeros). `render_money()` caches the previous value and only calls `draw_money()` when the balance has changed, avoiding unnecessary VRAM writes.

### `load_card_at()` — Card rendering

```c
void load_card_at(uint8_t x, uint8_t y, uint8_t val, uint8_t suit);
```

- `val`: 1–13 (Ace=1, 2–10, Jack=11, Queen=12, King=13)
- `suit`: 0=hearts, 1=spades, 2=diamonds, 3=clubs

Checks if the card is already in VRAM (via bitmask), loads it with `set_bkg_data()` if not, then positions the 4 card tiles (2×2 grid) at coordinates `(x, y)`.

### `wait_pad_release()` — Input debounce

```c
void wait_pad_release(void);
```

Spins until all joypad buttons are released. Essential after a validation press (`J_A`) to prevent the same button press from being read again on the next screen.

### `input_handler()` — Name input

```c
// src/utils/get_input.h
#define INPUT_BUFFER  11    // Max 10 chars + '\0'
#define ALPHABET1     "a b c d e f g h i"
#define ALPHABET2     "j k l m n o p q r"
#define ALPHABET3     "s t u v w x y z <"

void input_handler(char buffer[INPUT_BUFFER]);
```

Displays a 3×9 virtual keyboard navigable with the D-pad. `<` = backspace. `A` = confirm letter. `START` = finish input.

---

## 13. Transitions and Visual Effects

```c
// src/utils/transitions.h

// Erase screen top to bottom (replace tiles with blanks row by row)
void transition_wipe_down(void);

// Erase screen bottom to top
void transition_wipe_up(void);

// Invert the palette N times (white flash effect)
void transition_flash(uint8_t times);

// Oscillate SCX_REG horizontally (screen shake effect)
void transition_shake(void);
```

These effects are purely visual and do not modify any game state. Usage patterns:
- `wipe_down` / `wipe_up`: screen transitions (bet selection → game round)
- `flash`: win, double-down, jackpot
- `shake`: loss, bust, game over

---

## 14. Hardware Initialization

### `init()` — `src/init/init_game_boy.c`

Configures Game Boy hardware registers:
- 8×8 sprite mode
- Enables the background layer
- Configures VBL interrupt

### `init_font()` — `src/init/init_font.c`

Loads the character font into VRAM starting at tile index 0 (tiles 0–95). This covers ASCII characters 32 (' ') through 127. The font layout maps directly to ASCII, so rendering character `c` uses tile index `c - 32`.

---

## 15. Game Catalogue

### Menu (`menu.c` — 122 lines)

**Signature**: `uint8_t menu(bank_t *player_bank)`

Internal structure:
- Animated banner sprites (twinkling lights via `menu_banner.h`)
- Scrolling display: previous game / current game (with `->`) / next game
- UP/DOWN navigation with wraparound
- Returns the selected game index when A is pressed

Key variables:
```c
uint8_t game_idx = 0;              // Current position in game_tab[]
uint8_t last_idx = MIN_GAME_IDX;   // Last drawn index (change detection)
```

The `MIN_GAME_IDX` sentinel (defined in `menu.h`) triggers wraparound to `GAME_TAB_SIZE - 1`.

### Tax Money (`menu.c`)

```c
uint8_t tax_money(bank_t *player_bank) {
  player_bank->money += 10;
  return 0;   // Immediately return to menu
}
```

Easter egg: adds 10 to balance, returns to menu instantly.

### Black Jack (`black_jack.c` — 411 lines)

Full implementation:
- Bet selection (5 / 10 / 25 / 50) with fund validation
- 2+2 card deal with animation and card flip sound
- Score calculation handling Ace as 11 or 1
- Actions: Hit (A), Stand (B), Double Down (START, first 2 cards only)
- Dealer strategy: draws until score ≥ 17
- Outcomes: Blackjack (pays 3:2), Bust, Push, Win, Lose
- Dealer face sprite with animated eyes

Score calculation:
```c
static uint8_t calc_score(uint8_t *hand, uint8_t count) {
  uint8_t score = 0, aces = 0, i;
  for (i = 0; i < count; i++) {
    if (hand[i] == 1)       { aces++; score += 11; }
    else if (hand[i] >= 10) score += 10;
    else                    score += hand[i];
  }
  while (score > 21 && aces > 0) { score -= 10; aces--; }
  return score;
}
```

### Slot Machine (`slot.c` — 492 lines)

- 4 bet options: 5, 10, 25, 50
- 3 reels with acceleration / deceleration animation
- Win combination detection
- Border animation on jackpot
- Jackpot and loss sounds

### Roulette (`roulette.c` — 331 lines)

- 7 bet types: Red / Black / Green / Odd / Even / Low / High
- Adjustable bet amount (5–500 by step of 5)
- Spinning wheel animation (37 outcomes: 0–36)
- Ball physics simulation
- Background melody (13-note sequence)
- Payouts: ×2 color, ×35 green (zero), ×2 odd/even/low/high

Bet type constants:
```c
#define BET_RED    0   #define PAY_COLOR  2
#define BET_BLACK  1   #define PAY_GREEN  35
#define BET_GREEN  2   #define PAY_ODDEVEN 2
#define BET_ODD    3   #define PAY_LOWHIGH 2
#define BET_EVEN   4
#define BET_LOW    5
#define BET_HIGH   6
```

### Poker (`poker.c` — 312 lines)

- 5-card draw poker
- Deal phase → hold/redraw phase
- Hand evaluation: Royal Flush (9) → Nothing (0)

Hand rank constants:
```c
#define HAND_NOTHING       0
#define HAND_JACKS_BETTER  1
#define HAND_TWO_PAIR      2
#define HAND_THREE_KIND    3
#define HAND_STRAIGHT      4
#define HAND_FLUSH         5
#define HAND_FULL_HOUSE    6
#define HAND_FOUR_KIND     7
#define HAND_STRAIGHT_FLUSH 8
#define HAND_ROYAL_FLUSH   9
```

### Tetris (`tetris.c` — 351 lines)

- 10×16 board
- 7 tetromino types (I, O, T, S, Z, J, L)
- Gravity, rotation, DAS (Delayed Auto-Shift: delay=16, repeat=6 frames)
- Line clearing with scoring
- Level-based speed increase
- Game over detection

### Leaderboard (`leaderboard.c` — 68 lines)

- Reads all 8 save slots
- Sorts by money descending (bubble sort)
- Displays the ranking
- Shows "NO SAVES YET" if no save found

### Cheat Code / Player Name (`cheat.c` — 52 lines)

- Name input via `input_handler()`
- Secret codes:
  - `"tetris"` → launch Tetris directly (returns `BLAC_JACK` index as redirect)
  - `"epitech"` → +42 money
- Saves or loads the matching player profile

---

## 16. Full Execution Flow

```
Game Boy power on
       │
       ▼
   main()
       │
       ├- Set gray palette (BGP_REG = 0xE4)
       ├- Load load_screen tiles + tile map
       ├- Enable LCD, activate wave interrupt
       ├- Wait for J_START
       ├- Disable wave interrupt, reset SCX_REG
       ├- init() + init_font()
       ├- save_load("Player", &bank)
       │
       ▼
   Infinite main loop  ◄-----------------------------------┐
       │                                                    │
       ├- game_tab[game_idx].game(&bank)                    │
       │         │                                          │
       │         ▼                                          │
       │    [Game executes]                                  │
       │    - Reads joypad()                                │
       │    - Modifies bank.money                           │
       │    - Calls wait_vbl_done() every frame             │
       │    - Returns next_game_idx                         │
       │         │                                          │
       ├- game_idx = next_game_idx                          │
       ├- save_write(&bank)                                 │
       └- wait_vbl_done() ---------------------------------┘
```

### Typical session flow

```
Menu [idx=0]
  │  Player navigates UP/DOWN, selects Black Jack
  │  Presses A → returns BLAC_JACK (2)
  ▼
Black Jack [idx=2]
  │  Player plays several hands
  │  Presses SELECT → returns MENU (0)
  ▼
save_write() → persists updated balance
  ▼
Menu [idx=0]
  └- ... (infinite)
```

---

## 17. VRAM Memory Map

```
VRAM 0x8000–0x97FF (Tile Data — 384 tiles × 16 bytes each)
┌-------┐
│ Tiles   0– 95 : Font (ASCII 32–127)                     │
│ Tiles  96–127 : UI assets (casino borders, chips, etc.) │
│ Tiles 128–131 : Suit symbols ♥♠♦♣ (Black Jack)          │
│ Tiles 132–179 : Current game assets (varies by game)    │
│ Tiles 180–199 : Dealer sprite + eyes (Black Jack)       │
│ Tiles 200–211 : Slot machine symbols                    │
│ Tiles 204–204 : Roulette ball (reuses slot range)       │
│ Tiles 212–383 : Card VRAM cache (~42 cards max)         │
└-------┘

BKG Tile Map 0x9800–0x9BFF (32×32 = 1024 entries)
┌-------┐
│ Visible viewport : 20×18 tiles (160×144 pixels)         │
│ Off-screen area  : scrolling buffer / overflow tiles    │
└-------┘

External RAM 0xA000–0xBFFF (battery-backed, MBC5)
┌-------┐
│ 0xA000–0xA00F : Save slot 0 (16 bytes)                  │
│ 0xA010–0xA01F : Save slot 1                             │
│ ...                                                     │
│ 0xA070–0xA07F : Save slot 7                             │
│ 0xA080–0xBFFF : Unused                                  │
└-------┘
```

---

## 18. Code Conventions

### Naming

| Category | Convention | Example |
|----------|-----------|---------|
| Types | `snake_case` + `_t` suffix | `bank_t`, `game_t`, `save_slot_t` |
| Functions | `snake_case` | `draw_text()`, `save_write()` |
| Constants / macros | `UPPER_SNAKE_CASE` | `SAVE_MAGIC`, `CLEAR_BKG` |
| Local variables | `snake_case` | `game_idx`, `player_bank` |
| Asset tile count | `<asset>_TILE_COUNT` | `c_A_coeur_TILE_COUNT` |

### Common idioms

**Joypad reading with edge detection (one-shot press):**
```c
uint8_t keys = 0, prev_keys, keys_pressed;

// Inside the game loop:
prev_keys    = keys;
keys         = joypad();
keys_pressed = (keys ^ prev_keys) & keys;  // Bits set only on the frame of press

if (keys_pressed & J_A) { /* one-shot action */ }
if (keys & J_DOWN)       { /* held action */     }
```

**VBL synchronization (mandatory at end of every frame):**
```c
wait_vbl_done();
```

**Rendering a single character tile:**
```c
// Character 'X' (ASCII 88) → tile index 88 - 32 = 56
set_bkg_tile_xy(col, row, 'X' - 32);
```

**Clean slate between games:**
```c
CLEAR_BKG;           // fill_bkg_rect(0, 0, 32, 32, 0)
reset_card_memory(); // Invalidate card VRAM cache
audio_init();        // Silence all audio channels
```

**Preventing input bleed between screens:**
```c
wait_pad_release();  // After any J_A / J_B confirmation
```

### Code formatting

The project uses **clang-format** for consistency. Run `make format` before every commit. The CI check `make check_format` will fail if any file is not properly formatted.

---

# Part II — Implementing a New Game

---

## 19. Overview of the Integration Process

Adding a new game requires exactly **4 steps**:

1. Create `src/games/my_game.h` — constants and function declaration
2. Create `src/games/my_game.c` — game implementation
3. Register in `src/games/games.h` and `src/games/games.c` — dispatcher
4. Build and test

Because the Makefile uses wildcard source discovery (`$(wildcard src/games/*.c)`), your new `.c` file is **automatically picked up** by the build system without any Makefile modification.

The game function signature is fixed:

```c
uint8_t my_game(bank_t *player_bank);
```

It receives the player's context (name, balance) and returns the index of the next game to run. Return `MENU` (0) to go back to the main menu.

---

## 20. Step 1 — Create the Game Header

Create `src/games/my_game.h`. This file should contain:
- An include guard
- All `#define` constants specific to this game (layout positions, game parameters)
- The public function declaration

```c
// src/games/my_game.h

#ifndef MY_GAME_H
#define MY_GAME_H

// Screen is 20 columns × 18 rows (tile coordinates)
#define ROW_TITLE     1
#define ROW_BALANCE   2
#define ROW_SEPARATOR 3
#define ROW_RESULT    8
#define ROW_HINT     16

#define MY_BET_MIN    5
#define MY_BET_MAX  100
#define MY_BET_STEP   5

// -- Function declaration 
uint8_t my_game(bank_t *player_bank);

#endif
```

**Design guidelines for the header:**

- Keep all magic numbers in the header, not scattered in the `.c` file
- Row/column positions as named constants make layout changes trivial
- Parameters such as bet min/max/step belong here for easy tuning
- Keep the header free of implementation details — it is an interface contract

---

## 21. Step 2 — Create the Game Source File

Create `src/games/my_game.c`. The structure below reflects the pattern used consistently across all existing games.

```c
// src/games/my_game.c

#include <gb/gb.h>   // GBDK hardware access (joypad, wait_vbl_done, etc.)
#include <rand.h>    // rand() — Game Boy pseudo-random number generator

// Project-level includes
#include "../audio/audio.h"
#include "../casino.h"
#include "../utils/transitions.h"
#include "../utils/utils.h"
#include "games.h"
#include "my_game.h"

// ---------------------------
// Private helpers
// ---------------------------

static void draw_ui(bank_t *player_bank) {
  draw_text(3, ROW_TITLE, "MY GAME");
  draw_text(0, ROW_BALANCE, player_bank->name);
  draw_money(player_bank->money, 12, ROW_BALANCE);
  draw_text(0, ROW_SEPARATOR, "--------------------");
  draw_text(0, ROW_HINT, "A:PLAY  SEL:QUIT");
}

// ---------------------------
// Public game function — called by the dispatcher in main()
// ---------------------------

uint8_t my_game(bank_t *player_bank) {
  uint8_t keys = 0, prev_keys, keys_pressed;
  uint8_t bet = MY_BET_MIN;
  uint8_t result;

  // -- Initialization ------
  audio_init();     // Silence all channels from previous game
  CLEAR_BKG;        // Blank the background tile map

  draw_ui(player_bank);

  wait_pad_release();  // Prevent input bleed from the menu screen

  // -- Main game loop ------
  for (;;) {
    prev_keys    = keys;
    keys         = joypad();
    keys_pressed = (keys ^ prev_keys) & keys;

    // Return to menu
    if (keys_pressed & J_SELECT)
      return MENU;

    // Adjust bet
    if (keys_pressed & J_UP && bet < MY_BET_MAX)
      bet += MY_BET_STEP;
    if (keys_pressed & J_DOWN && bet > MY_BET_MIN)
      bet -= MY_BET_STEP;

    // Play a round
    if (keys_pressed & J_A) {
      if (bet > player_bank->money) {
        draw_text(1, ROW_RESULT, "NOT ENOUGH FUNDS");
      } else {
        play_confirm();
        transition_wipe_down();

        // -- Core game logic -----------------------------------------------
        // initrand(DIV_REG) seeds the RNG from the hardware divider
        // register — its value depends on exact timing of player input,
        // providing genuine unpredictability.
        initrand(DIV_REG);
        result = (uint8_t)rand() & 1;  // 0 or 1

        // -- Outcome -----
        if (result) {
          player_bank->money += bet;
          transition_flash(3);
          play_win();
          draw_text(4, ROW_RESULT, "YOU WIN!");
        } else {
          if (player_bank->money >= bet)
            player_bank->money -= bet;
          else
            player_bank->money = 0;
          transition_shake();
          play_lose();
          draw_text(3, ROW_RESULT, "YOU LOSE!");
        }

        draw_money(player_bank->money, 12, ROW_BALANCE);

        // -- Game over check -----------------------------------------------
        if (player_bank->money == 0) {
          play_game_over();
          draw_text(2, ROW_RESULT + 2, "NO MORE FUNDS");
          delay(2500);
          return MENU;
        }

        // -- Continue prompt -----------------------------------------------
        draw_text(0, ROW_HINT, "A:RETRY  SEL:QUIT");
        wait_pad_release();

        // Wait for A (retry) or SELECT (menu)
        while (1) {
          keys = joypad();
          if (keys & J_A) {
            play_confirm();
            CLEAR_BKG;
            draw_ui(player_bank);
            wait_pad_release();
            break;
          }
          if (keys & J_SELECT)
            return MENU;
          wait_vbl_done();
        }
      }
    }

    wait_vbl_done();  // Must be called every frame — synchronizes with VBL
  }
}
```

---

## 22. Step 3 — Register the Game in the Dispatcher

You need to edit two files: `games.h` and `games.c`.

### Edit `src/games/games.h`

Add the function declaration and a new index constant:

```c
// src/games/games.h

#ifndef GAMES_H
#define GAMES_H

#include <stdint.h>
#include "../casino.h"

// -- Function declarations -------------------------------------------------
uint8_t black_jack(bank_t *player_bank);
uint8_t slot_machine(bank_t *player_bank);
uint8_t roulette(bank_t *player_bank);
uint8_t poker(bank_t *player_bank);
uint8_t leaderboard(bank_t *player_bank);
uint8_t menu(bank_t *player_bank);
uint8_t tax_money(bank_t *player_bank);
uint8_t cheat_code(bank_t *player_bank);
uint8_t my_game(bank_t *player_bank);          // ← ADD THIS LINE

// -- Game index constants 
#define MENU         0
#define SLOT         MENU + 1
#define BLAC_JACK    SLOT + 1
#define ROULETTE     BLAC_JACK + 1
#define POKER        ROULETTE + 1
#define TAX_MONEY    POKER + 1
#define CHEAT_CODE   TAX_MONEY + 1
#define LEADERBOARD  CHEAT_CODE + 1
#define MY_GAME      LEADERBOARD + 1             // ← ADD THIS LINE

// -- Dispatch table ------
typedef struct game_s {
  char     name[16];
  uint8_t  name_size;
  uint8_t (*game)(bank_t *);
} game_t;

extern const game_t game_tab[];
extern const uint8_t GAME_TAB_SIZE;

#endif
```

**Important**: the numeric value of each constant is derived from the one before it. The new constant `MY_GAME` automatically gets the value 8 (one past LEADERBOARD). This chain of additions means the order of defines must match the order of entries in `game_tab[]`.

### Edit `src/games/games.c`

Add the new entry to the dispatch table and increment `GAME_TAB_SIZE`:

```c
// src/games/games.c

#include "games.h"
#include "../casino.h"

#define GAME_NAME(a) a, sizeof(a)

const game_t game_tab[] = {
  {GAME_NAME("Menu"),        menu},
  {GAME_NAME("Slots"),       slot_machine},
  {GAME_NAME("Black Jack"),  black_jack},
  {GAME_NAME("Roulette"),    roulette},
  {GAME_NAME("Poker"),       poker},
  {GAME_NAME("Tax money"),   tax_money},
  {GAME_NAME("Player name"), cheat_code},
  {GAME_NAME("Scores"),      leaderboard},
  {GAME_NAME("My Game"),     my_game},    // ← ADD THIS LINE
};

const uint8_t GAME_TAB_SIZE = 9;          // ← INCREMENT THIS
```

The string `"My Game"` is what appears in the menu selector. It must fit in 16 bytes (15 chars + null). `GAME_NAME` uses `sizeof` which includes the null terminator, so `name_size` will be 8 for "My Game" (7 chars + null).

The menu wraps around using `GAME_TAB_SIZE`, so updating it is mandatory — otherwise the last entry is unreachable.

---

## 23. Step 4 — Build and Test

```bash
# Full clean rebuild (recommended when adding new files)
make re

# Or incremental build
make

# Launch in emulator
make launch
```

If the build fails with an undefined symbol error for `my_game`, verify:
1. The declaration in `games.h` matches the signature in `my_game.c` exactly
2. The `my_game.c` file is in `src/games/` (the wildcard path)
3. You included `games.h` in your `.c` file

If the game does not appear in the menu, verify:
1. `GAME_TAB_SIZE` was incremented in `games.c`
2. The new entry is the last in `game_tab[]` (matching the `MY_GAME` index)

---

## 24. Input Handling Patterns

The Game Boy joypad provides 8 buttons. The GBDK `joypad()` function returns a bitmask of currently pressed buttons.

### Pattern 1 — One-shot press (action fires once per press)

Use this for: confirming bets, drawing cards, any action that must not repeat while held.

```c
uint8_t keys = 0, prev_keys, keys_pressed;

// In the game loop:
prev_keys    = keys;
keys         = joypad();
keys_pressed = (keys ^ prev_keys) & keys;

if (keys_pressed & J_A)      { /* fires exactly once per press */ }
if (keys_pressed & J_SELECT) { return MENU; }
```

### Pattern 2 — Held button (action repeats while held)

Use this for: scrolling menus, adjusting values, continuous movement.

```c
keys = joypad();
if (keys & J_UP)   { value++; }
if (keys & J_DOWN) { value--; }
```

**Warning**: without a delay or frame limiter, a held button fires ~60 times per second (once per `wait_vbl_done()` call). For menu navigation, use one-shot detection and call `wait_pad_release()` after each navigation step.

### Pattern 3 — Blocking wait for a specific button

Use this for: "press A to continue" prompts.

```c
wait_pad_release();   // Ensure the button is not already held
while (!(joypad() & J_A))
  wait_vbl_done();
```

### Pattern 4 — Exit guard (SELECT always returns to menu)

Every game must handle SELECT to return to the menu. Place this check at the top of the input handling block:

```c
if (keys_pressed & J_SELECT)
  return MENU;
```

### Randomness

Seed the RNG immediately before generating a random value, using the hardware divider register as entropy:

```c
initrand(DIV_REG);          // Seed once, at the moment the player acts
uint8_t r = (uint8_t)rand() % N;   // Random number in [0, N)
```

`DIV_REG` (0xFF04) increments at ~16384 Hz regardless of game code, so its value at the moment of player input is highly unpredictable — this is the standard entropy source on Game Boy.

---

## 25. Drawing to Screen

### Screen coordinate system

The screen is **20 columns × 18 rows** of 8×8 tiles. Column 0 is leftmost, row 0 is topmost.

```
(0,0)------------------(19,0)
  │                        │
  │   160×144 px screen    │
  │   = 20×18 tiles        │
  │                        │
(0,17)-----------------(19,17)
```

### Drawing text strings

```c
draw_text(uint8_t col, uint8_t row, const char *str);
```

```c
draw_text(3, 1,  "MY GAME");           // Title, centered roughly
draw_text(0, 16, "A:PLAY  SEL:QUIT");  // Controls hint at bottom
```

The string must consist of printable ASCII characters (32–127). Each character occupies one tile.

### Drawing numeric values

```c
draw_money(uint32_t money, uint8_t col, uint8_t row);
```

Renders the number right-aligned with leading spaces suppressed. The number `1234` at column 10 would occupy columns 10–13.

### Placing individual tiles

```c
// Place a specific tile index at (col, row)
set_bkg_tile_xy(uint8_t col, uint8_t row, uint8_t tile_id);

// Place tile for ASCII character 'X'
set_bkg_tile_xy(5, 3, 'X' - 32);
```

### Clearing the screen

Always clear before drawing a new screen to avoid tile ghosting from the previous game:

```c
CLEAR_BKG;   // Fills entire 32×32 background tilemap with tile 0 (blank)
```

### Drawing a horizontal separator line

```c
draw_text(0, 3, "--------------------");  // 20 dashes = full width
```

### Displaying cards

```c
// Load and render a card at tile position (col, row)
// val: 1–13 (1=Ace, 11=Jack, 12=Queen, 13=King)
// suit: 0=hearts, 1=spades, 2=diamonds, 3=clubs
// Each card occupies a 2×2 tile block (16×16 pixels)
load_card_at(uint8_t col, uint8_t row, uint8_t val, uint8_t suit);

// Reset the VRAM card cache between rounds
reset_card_memory();
```

---

## 26. Using Audio

### At game start — always call audio_init()

```c
audio_init();   // Silences all channels, prevents audio bleed from previous game
```

### Using preset sounds

```c
play_select();      // Navigation beep
play_confirm();     // Accept/confirm action
play_win();         // Player wins
play_lose();        // Player loses
play_game_over();   // No more money
```

### Playing custom notes on CH1 (melody / lead)

```c
// Short high note — e.g., card draw
ch1_play(NOTE_G4_LO, NOTE_G4_HI, 2, 8, 12, 0, 3);

// Sustained win chord
ch1_play(NOTE_C5_LO, NOTE_C5_HI, 2, 30, 15, 0, 1);
delay(300);
ch1_play(NOTE_E5_LO, NOTE_E5_HI, 2, 30, 15, 0, 1);
```

### Playing harmony on CH2

CH2 layers on top of CH1. Use it for chords:

```c
ch1_play(NOTE_C5_LO, NOTE_C5_HI, 2, 20, 15, 0, 1);
ch2_play(NOTE_E5_LO, NOTE_E5_HI, 2, 20, 13, 0, 1);
```

### Playing percussion on CH4 (noise)

```c
// Soft thud — e.g., chip placed
ch4_play(6, 0, 2, 7, 0, 20);

// Card flip — sharp noise burst
ch4_play(8, 0, 1, 5, 0, 32);

// Heavy loss sound
ch4_play(15, 0, 3, 3, 0, 60);
```

CH4 parameters: `(volume, envelope_dir, envelope_step, shift, lfsr_width, length)`.

### Playing a melody sequence

```c
static const uint8_t melody_lo[] = {NOTE_C4_LO, NOTE_E4_LO, NOTE_G4_LO, NOTE_C5_LO};
static const uint8_t melody_hi[] = {NOTE_C4_HI, NOTE_E4_HI, NOTE_G4_HI, NOTE_C5_HI};

for (uint8_t i = 0; i < 4; i++) {
  ch1_play(melody_lo[i], melody_hi[i], 2, 15, 12, 0, 2);
  delay(150);
}
```

`delay(ms)` is a blocking busy-wait. During a `delay()`, no input is read and the game does not respond. Keep delay durations short (< 500 ms) unless it is intentional (e.g., a 2.5-second game over pause).

---

## 27. Handling Player Money

### Reading the balance

```c
uint32_t current = player_bank->money;
```

### Adding winnings

```c
player_bank->money += bet;               // Simple win: 1:1
player_bank->money += bet + (bet >> 1);  // 3:2 payout (Blackjack)
player_bank->money += bet * multiplier;  // N:1 payout
```

### Subtracting a bet

Always guard against underflow since `money` is unsigned:

```c
if (player_bank->money >= bet)
  player_bank->money -= bet;
else
  player_bank->money = 0;
```

### Checking if the player can afford a bet

```c
if (bet > player_bank->money) {
  draw_text(1, ROW_RESULT, "NOT ENOUGH FUNDS");
  // Do not proceed with the round
}
```

### Validating that a bet option is available

When offering multiple bet tiers, skip options the player cannot afford:

```c
const uint8_t bets[] = {5, 10, 25, 50};
uint8_t selected = 0;

// Start on the highest affordable bet
for (uint8_t i = 0; i < 4; i++) {
  if (bets[i] <= player_bank->money)
    selected = i;
}
```

### Game over condition

Every game must check for zero balance and handle it gracefully:

```c
if (player_bank->money == 0) {
  transition_wipe_down();
  play_game_over();
  draw_text(2, 8,  "GAME OVER!");
  draw_text(1, 10, "NO MORE FUNDS");
  delay(2500);
  return MENU;
}
```

### Refreshing the balance display

After every bet or payout, redraw the money:

```c
draw_money(player_bank->money, 12, ROW_BALANCE);
```

The save system is handled automatically by `main()` after every game function returns. You do not need to call `save_write()` inside a game.

---

## 28. Adding Custom Assets

If your game needs graphics beyond the existing tile set, you can add custom assets.

### Step 1 — Prepare the PNG

Place your source image in `asset/`. The image must use the Game Boy palette (4 shades of gray, or a 4-color palette that will be mapped). Each tile is 8×8 pixels.

### Step 2 — Convert to C with png2asset

```bash
# From the project root (with GBDK environment active)
gbdk/bin/png2asset asset/my_asset.png -c src/assets/my_asset.c -noflip
```

This generates `src/assets/my_asset.c` and `src/assets/my_asset.h`. The Makefile picks up `src/assets/*.c` automatically.

### Step 3 — Include and load the asset header

In your game's `.c` file:

```c
#include "../assets/my_asset.h"
```

### Step 4 — Load tiles into VRAM at runtime

```c
// Load my_asset_TILE_COUNT tiles into VRAM starting at index 132
// (safe zone after font and UI tiles — verify the VRAM map for your game)
set_bkg_data(132, my_asset_TILE_COUNT, my_asset_tiles);
```

### Step 5 — Place tiles on screen

```c
// If the asset has a tilemap (for multi-tile backgrounds):
set_bkg_tiles(0, 0, my_asset_WIDTH, my_asset_HEIGHT, my_asset_map);

// Or place individual tiles manually:
set_bkg_tile_xy(col, row, 132 + tile_index);
```

### VRAM budget awareness

You have 384 tile slots total. The font alone takes 96. Reserve your tile range carefully to avoid overwriting the font or another game's assets. Check the VRAM map in Section 17 and pick an unused range.

### Working with the existing casino asset tiles

If your game fits the casino aesthetic, use the pre-built UI tiles rather than creating new ones:

```c
#include "../assets/casino_asset.h"

// In your game initialization:
load_casino_tiles();    // Loads tiles 96–109 into VRAM
draw_casino_bg();       // Draws the full felt background
draw_casino_hdiv(5);    // Draws a horizontal divider at row 5
```

---

## 29. Complete Annotated Example — Coin Flip Game

This is a fully functional, minimal game that demonstrates every integration point. It implements a simple coin flip where the player bets on heads or tails.

### `src/games/coin_flip.h`

```c
#ifndef COIN_FLIP_H
#define COIN_FLIP_H

// Screen layout — tile coordinates (col, row)
#define CF_ROW_TITLE    1
#define CF_ROW_BALANCE  2
#define CF_ROW_SEP      3
#define CF_ROW_BET      5
#define CF_ROW_SIDE     7
#define CF_ROW_RESULT  10
#define CF_ROW_HINT    16

// Game parameters
#define CF_BET_MIN   5
#define CF_BET_MAX 100
#define CF_BET_STEP  5

// Sides
#define CF_HEADS 0
#define CF_TAILS 1

uint8_t coin_flip(bank_t *player_bank);

#endif
```

### `src/games/coin_flip.c`

```c
#include <gb/gb.h>
#include <rand.h>

#include "../audio/audio.h"
#include "../casino.h"
#include "../utils/transitions.h"
#include "../utils/utils.h"
#include "coin_flip.h"
#include "games.h"

// --- Helpers ----------------

static void draw_header(bank_t *player_bank) {
  draw_text(4,  CF_ROW_TITLE,   "COIN FLIP");
  draw_text(0,  CF_ROW_BALANCE, player_bank->name);
  draw_money(player_bank->money, 12, CF_ROW_BALANCE);
  draw_text(0,  CF_ROW_SEP,     "--------------------");
}

static void draw_bet_display(uint32_t bet) {
  draw_text(0, CF_ROW_BET, "BET: ");
  draw_money(bet, 5, CF_ROW_BET);
  draw_text(0, CF_ROW_BET + 1, "UP/DOWN: CHANGE BET ");
}

static void draw_side_display(uint8_t side) {
  draw_text(0, CF_ROW_SIDE, "SIDE: ");
  draw_text(6, CF_ROW_SIDE, side == CF_HEADS ? "HEADS " : "TAILS ");
  draw_text(0, CF_ROW_SIDE + 1, "LEFT/RIGHT: TOGGLE  ");
}

// --- Main function ----------

uint8_t coin_flip(bank_t *player_bank) {
  uint8_t keys = 0, prev_keys, keys_pressed;
  uint32_t bet = CF_BET_MIN;
  uint8_t side = CF_HEADS;
  uint8_t flip_result;

  // -- Setup ---------------
  audio_init();
  CLEAR_BKG;

  draw_header(player_bank);
  draw_bet_display(bet);
  draw_side_display(side);
  draw_text(0, CF_ROW_HINT, "A:FLIP    SEL:QUIT");

  wait_pad_release();

  // -- Main loop -----------
  for (;;) {
    prev_keys    = keys;
    keys         = joypad();
    keys_pressed = (keys ^ prev_keys) & keys;

    // Always-available: exit to menu
    if (keys_pressed & J_SELECT)
      return MENU;

    // Adjust bet with UP/DOWN
    if (keys_pressed & J_UP) {
      if (bet < CF_BET_MAX && bet + CF_BET_STEP <= player_bank->money) {
        bet += CF_BET_STEP;
        play_select();
        draw_bet_display(bet);
      }
    }
    if (keys_pressed & J_DOWN) {
      if (bet > CF_BET_MIN) {
        bet -= CF_BET_STEP;
        play_select();
        draw_bet_display(bet);
      }
    }

    // Toggle side with LEFT/RIGHT
    if (keys_pressed & J_LEFT || keys_pressed & J_RIGHT) {
      side ^= 1;  // Toggle between CF_HEADS and CF_TAILS
      play_select();
      draw_side_display(side);
    }

    // Flip the coin on A
    if (keys_pressed & J_A) {
      if (bet > player_bank->money) {
        draw_text(1, CF_ROW_RESULT, "NOT ENOUGH FUNDS!   ");
      } else {
        play_confirm();

        // Seed RNG from hardware divider — player's reaction time is the entropy
        initrand(DIV_REG);
        flip_result = (uint8_t)rand() & 1;

        // Animate: clear result area, show suspense text
        draw_text(0, CF_ROW_RESULT,     "                    ");
        draw_text(0, CF_ROW_RESULT + 1, "                    ");
        draw_text(3, CF_ROW_RESULT,     "FLIPPING...");
        delay(600);

        // Show outcome
        draw_text(3, CF_ROW_RESULT, flip_result == CF_HEADS ? "  HEADS!   " : "  TAILS!   ");

        if (flip_result == side) {
          // Win
          player_bank->money += bet;
          transition_flash(2);
          play_win();
          draw_text(4, CF_ROW_RESULT + 1, "YOU WIN!  ");
        } else {
          // Lose
          if (player_bank->money >= bet)
            player_bank->money -= bet;
          else
            player_bank->money = 0;
          transition_shake();
          play_lose();
          draw_text(3, CF_ROW_RESULT + 1, "YOU LOSE! ");
        }

        // Refresh balance display
        draw_money(player_bank->money, 12, CF_ROW_BALANCE);

        // Game over?
        if (player_bank->money == 0) {
          play_game_over();
          draw_text(2, CF_ROW_RESULT + 2, "NO MORE FUNDS");
          delay(2500);
          return MENU;
        }

        // Continue prompt
        draw_text(0, CF_ROW_HINT, "A:RETRY   SEL:QUIT  ");
        wait_pad_release();

        while (1) {
          keys = joypad();
          if (keys & J_A) {
            play_confirm();
            // Redraw UI for next round
            CLEAR_BKG;
            draw_header(player_bank);
            draw_bet_display(bet);
            draw_side_display(side);
            draw_text(0, CF_ROW_HINT, "A:FLIP    SEL:QUIT");
            wait_pad_release();
            keys = 0;  // Reset to avoid edge-detection issues
            break;
          }
          if (keys & J_SELECT)
            return MENU;
          wait_vbl_done();
        }
      }
    }

    wait_vbl_done();
  }
}
```

### Add to `src/games/games.h`

```c
uint8_t coin_flip(bank_t *player_bank);   // Add declaration

#define COIN_FLIP  LEADERBOARD + 1        // Add index constant (= 8)
```

### Add to `src/games/games.c`

```c
{GAME_NAME("Coin Flip"), coin_flip},      // Add entry

const uint8_t GAME_TAB_SIZE = 9;          // Increment size
```

### Build and run

```bash
make re && make launch
```

Navigate the menu to "Coin Flip", press A. The game will appear, allowing the player to flip coins with adjustable bet amounts and side selection.

---

## 30. Common Pitfalls

### Forgetting `wait_vbl_done()` in the game loop

Every iteration of a game loop must end with `wait_vbl_done()`. Without it, the CPU spins at full speed, VRAM writes occur during active rendering (causing graphical glitches), and input reads may be inconsistent.

```c
// Correct
for (;;) {
  keys = joypad();
  // ... game logic ...
  wait_vbl_done();   // ← mandatory
}
```

### Forgetting `wait_pad_release()` at screen transitions

If the player presses A to confirm a bet and you immediately show the result screen, the new screen's loop reads J_A as still held and fires again. Always debounce at screen boundaries:

```c
play_confirm();
// Draw next screen...
wait_pad_release();   // ← prevent input bleed
```

### Forgetting `audio_init()` at game start

Without `audio_init()`, audio channels from the previous game continue playing. Slots might still be playing win music when you enter Black Jack.

### Stack overflow from large local arrays

On Game Boy, the stack is small. Large arrays declared locally inside a function can overflow the stack and corrupt memory silently.

```c
// WRONG — 200 bytes on the stack, may overflow
uint8_t board[10][20];

// CORRECT — static storage, outside the stack
static uint8_t board[10][20];
```

Use `static` for any local buffer larger than ~20 bytes.

### VRAM corruption from tile index collisions

Loading tiles at an index that is already used by the font (0–95) will corrupt text rendering. Always verify your tile ranges against the VRAM map in Section 17.

```c
// WRONG — overwrites font tiles
set_bkg_data(0, my_asset_TILE_COUNT, my_asset_tiles);

// CORRECT — safe zone above the font
set_bkg_data(132, my_asset_TILE_COUNT, my_asset_tiles);
```

### Index constant out of sync with game_tab[]

If you add a constant `MY_GAME = LEADERBOARD + 1` but place the entry at position 3 in `game_tab[]`, the constant and the table are inconsistent. The dispatcher will call the wrong function when returning `MY_GAME`.

Always append new games to the **end** of both the constant chain and `game_tab[]`.

### Not incrementing `GAME_TAB_SIZE`

The menu uses `GAME_TAB_SIZE` to know when to wrap around. If you add a game without updating this constant, the last entry in the table is unreachable from the menu.

### Unsigned subtraction underflow

`bank_t.money` is `uint32_t`. Subtracting more than the current balance wraps around to a huge number instead of going negative.

```c
// WRONG
player_bank->money -= bet;   // wraps to ~4 billion if bet > money

// CORRECT
if (player_bank->money >= bet)
  player_bank->money -= bet;
else
  player_bank->money = 0;
```

### Using `delay()` inside the game loop for animations

`delay(ms)` is a blocking busy-wait. Any button presses during a `delay()` are lost. For animations that must remain responsive (e.g., a spinning reel the player can stop), use a frame counter instead:

```c
// Instead of delay(300):
uint8_t frame = 0;
while (frame < 18) {   // 18 frames ≈ 300 ms at 60fps
  // animate one step
  wait_vbl_done();
  frame++;
}
```

### Returning the wrong game index

Every game function must return a valid index into `game_tab[]`. Returning an index that does not exist causes the main loop to call a garbage function pointer and crash the ROM.

```c
// Always use the named constants, never raw numbers
return MENU;         // ← correct
return 0;            // ← works but fragile
return MY_GAME + 1;  // ← likely invalid, crashes
```
