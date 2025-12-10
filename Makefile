.PHONY: build run clean
.DEFAULT_GOAL := run

BUILD_DIR := build
CFLAGS := -Wall
PROGRAM_NAME := fvm-lc3
CC := gcc

$(BUILD_DIR):
	@mkdir -p $@

main: $(BUILD_DIR)
	$(CC) main.c -o $(BUILD_DIR)/$(PROGRAM_NAME) $(CFLAGS)

run: main
	./$(BUILD_DIR)/$(PROGRAM_NAME)

clean:
	rm -rf build/
