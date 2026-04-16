MAKE_FLAGS += -j

BIN_NAME := a.gb

SRC = $(wildcard src/*.c)
SRC += $(wildcard src/assets/*.c)
SRC += $(wildcard src/utils/*.c)
SRC += $(wildcard src/games/*.c)
SRC += $(wildcard src/init/*.c)

BUILD_DIR := .build

ifeq ($(shell uname -s),Darwin)
RETROARCH_CMD := /Applications/RetroArch.app/Contents/MacOS/RetroArch -L /Users/savalet/Library/Application\ Support/RetroArch/cores/gambatte_libretro.dylib $(BIN_NAME)
else
RETROARCH_CMD := retroarch -L ~/.config/retroarch/cores/gambatte_libretro.so $(BIN_NAME)
endif

include utils.mk

.PHONY: _start all
_start: all

CC = lcc

CFLAGS = -Wf--max-allocs-per-node50000

# call mk-profile release, SRC, additional CFLAGS
define mk-profile

NAME_$(strip $1) := $4
OBJ_$(strip $1) := $$($(strip $2):%.c=$$(BUILD_DIR)/$(strip $1)/%.o)

$$(BUILD_DIR)/$(strip $1)/%.o: %.c
	@ mkdir -p $$(dir $$@)
	@ $$(CC) $$(CFLAGS) $(strip $3) $$< -c -o $$@
	@ $$(LOG_TIME) "$$(C_GREEN) CC $$(C_PURPLE) $$(notdir $$@) $$(C_RESET)"

$$(NAME_$(strip $1)): $$(OBJ_$(strip $1))
	@ $$(CC) $$(OBJ_$(strip $1)) $$(LDFLAGS) $$(LDLIBS) $(strip $3) -o $$@
	@ $$(LOG_TIME) "$$(C_BLUE) LD $$(C_PURPLE) $$(notdir $$@) $$(C_RESET)"
	@ $$(LOG_TIME) "$$(C_GREEN) OK  Compilation finished $$(C_RESET)"

endef

$(eval $(call mk-profile, release, SRC, , $(BIN_NAME)))

all: $(NAME_release)

debug: CXXFLAGS += -D DEBUG_MODE
debug: all

fast: CFLAGS += -Wf--opt-code-speed
fast: all

small: CFLAGS += -Wf--opt-code-size
small: all

launch: all
launch:
	@ $(shell $(RETROARCH_CMD))

format:
	@ find ./ -name "*.c" -type f -exec clang-format -i {} ";"
	@ find ./ -name "*.h" -type f -exec clang-format -i {} ";"
	@ $(LOG_TIME) "$(C_BLUE) CF $(C_GREEN) Code formated  $(C_RESET)"

check_format:
	@ find ./ -name "*.c" -type f -exec clang-format --dry-run --Werror {} ";" 2>&1 | wc -m | grep 0 > /dev/null
	@ find ./ -name "*.h" -type f -exec clang-format --dry-run --Werror {} ";" 2>&1 | wc -m | grep 0 > /dev/null
	@ $(LOG_TIME) "$(C_BLUE) CF $(C_GREEN) Code formated  $(C_RESET)"

clean:
	@ $(RM) $(OBJ)
	@ $(LOG_TIME) "$(C_YELLOW) RM $(C_PURPLE) $(OBJ) $(C_RESET)"

fclean:
	@ $(RM) -r $(NAME_server) $(NAME_client) $(BUILD_DIR)
	@ $(LOG_TIME) "$(C_YELLOW) RM $(C_PURPLE) $(NAME_release) $(BUILD_DIR) \
		$(C_RESET)"

.NOTPARALLEL: re
re:	fclean all

.PHONY: all clean fclean re

PREFIX ?=
BINDIR ?= $(PREFIX)/bin
