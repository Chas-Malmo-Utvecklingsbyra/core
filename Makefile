CC ?= gcc

SRC_DIR := .

ifndef BUILD_DIR
$(error no BUILD_DIR specified)
endif

CVERSION ?= c90

CFLAGS := -std=$(CVERSION) $(SRC_DIR) -Wall -Wextra -MMD -MP -I.

LDFLAGS := -flto -Wl,--gc-sections

LIBS := 

SRC := $(shell find -L $(SRC_DIR) -type f -name '*.c')

FILES_TO_TEST := $(shell find -L $(SRC_DIR) -type f -name '*.test')

OBJ := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC))

DEP := $(OBJ:.o=.d)

BIN := server

all: $(BIN)
	@echo "Build complete."


$(BIN): $(OBJ)
	@$(CC) $(OBJ) -o $@ $(LDFLAGS) $(LIBS)


$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling $<..."
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(BIN)
	./$(BIN)

clean:
	@rm -rf $(BUILD_DIR) $(BIN)

-include $(DEP)

.PHONY: all run clean