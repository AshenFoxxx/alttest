CC = gcc
CFLAGS = -fPIC -Wall -Wextra -Iinclude

LIB_NAME = libalttest.so
BIN_NAME = alttest

SRC_LIB = src/lib.c
SRC_BIN = src/main.c

all: $(LIB_NAME) $(BIN_NAME)

$(LIB_NAME): $(SRC_LIB)
	$(CC) $(CFLAGS) -shared -o $@ $^

$(BIN_NAME): $(SRC_BIN) $(LIB_NAME)
	$(CC) $(CFLAGS) -o $@ $(SRC_BIN) -L. -lalttest

clean:
	rm -f $(LIB_NAME) $(BIN_NAME)
