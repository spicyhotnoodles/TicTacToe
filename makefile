# Makefile for your project

# Compiler and flags
CC = gcc
CFLAGS = -g -Wall -O2 -Wextra -Igame_server

# Sources and build
SRC = main.c game_server/communication.c game_server/hash.c
OBJ = $(SRC:.c=.o)
TARGET = build/server

# Default target
all: $(TARGET)

# Build executable
$(TARGET): $(OBJ)
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ $^

# Clean build artifacts
clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
