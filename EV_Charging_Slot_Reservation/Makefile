CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -pedantic -Iinclude
SRC     = $(wildcard src/*.c)
OBJ     = $(SRC:.c=.o)
TARGET  = ev_charging

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)

.PHONY: all clean
