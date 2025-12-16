CC      = gcc
CFLAGS  = -fPIC -Wall -Wextra -Iinclude
LDFLAGS = -lcurl

LIB_NAME  = libalttest.so
BIN_NAME  = alttest
TEST_BIN  = tests

SRC_LIB = src/lib.c
SRC_BIN = src/main.c

all: $(LIB_NAME) $(BIN_NAME)

$(LIB_NAME): $(SRC_LIB)
	$(CC) $(CFLAGS) -shared -o $@ $^ $(LDFLAGS)

$(BIN_NAME): $(SRC_BIN) $(LIB_NAME)
	$(CC) $(CFLAGS) -o $@ $(SRC_BIN) -L. -lalttest $(LDFLAGS)

$(TEST_BIN): $(LIB_NAME) src/test.c
	$(CC) $(CFLAGS) -o $@ src/test.c -L. -lalttest $(LDFLAGS)

tests: $(TEST_BIN)

clean:
	rm -f $(LIB_NAME) $(BIN_NAME) $(TEST_BIN)
