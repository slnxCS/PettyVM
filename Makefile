CC = g++
INCLUDES = -I. -I./src

CFLAGS = -Wall -Wextra -std=c++23 -Og -g $(INCLUDES)
LDFLAGS = -lm

BUILD_DIR = build
TARGET = $(BUILD_DIR)/main

SRCS = $(shell find . -type f \( -name "*.cpp" -o -name "*.c" -o -name "*.cc" \) -not -path './build/*')

OBJS = $(SRCS:%=$(BUILD_DIR)/%.o)

.PHONY: all clean clear run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.cc.o: %.cc
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean clear: 
	rm -rf $(BUILD_DIR)

run: $(TARGET)
	./$(TARGET)