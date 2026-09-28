CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Og -g -Iinclude
LDFLAGS = -lm

BUILD_DIR = build
TARGET = $(BUILD_DIR)/main

SRCS = $(shell find . -name '*.c' -not -path './build/*')
OBJS = $(patsubst ./%.c,$(BUILD_DIR)/%.o,$(SRCS))

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clear: 
	rm -rf $(BUILD_DIR)

run: $(TARGET)
	./$(TARGET)
