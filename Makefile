CC ?= gcc
C_VERSION ?= c99

ifndef OBJ_DIR
$(error no OBJ_DIR specified)
endif

SRC_FILES := $(shell find . -name "*.c")
SRC_FILES := $(filter-out ./tests/%,$(SRC_FILES))

INCLUDE_PATHS := .

CFLAGS = -std=$(C_VERSION) -Wall -Wextra -Werror -Wpedantic $(addprefix -I,$(INCLUDE_PATHS))

MY_OBJ_DIR = $(OBJ_DIR)/core

OBJS = $(patsubst %,$(MY_OBJ_DIR)/%,$(SRC_FILES))
OBJS := $(OBJS:.c=.o)

all: $(OBJS)
	@echo "Done compiling Core! Flags: $(CFLAGS)."

$(MY_OBJ_DIR)/%.o: %.c
	@echo "Compiling $< ..."
	@mkdir -p $(dir $@)
	@$(CC) -c $< $(CFLAGS) -o $@

clean_score:
	@rm -rf $(MY_OBJ_DIR)

.PHONY: all clean_score