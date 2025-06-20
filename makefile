CC = gcc
CFLAGS = -Wall -Wextra -g -Igame_server
SRC = main.c
OBJS = $(SRC:.c=.o)
TARGET = build/server

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
