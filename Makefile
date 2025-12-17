CC      = gcc
CFLAGS  = -fPIC -Wall -Wextra -Iinclude
LDFLAGS = -lcurl -ljansson



LIB_MAJOR = 1
LIB_VERSION = 1.0.0

LIB_REAL  = libalttest.so.$(LIB_VERSION)
LIB_SONAME = libalttest.so.$(LIB_MAJOR)
LIB_LINK  = libalttest.so

BIN_NAME  = alttest
TEST_BIN  = tests

SRC_LIB = src/lib.c
SRC_BIN = src/main.c

all: $(LIB_REAL) $(LIB_SONAME) $(LIB_LINK) $(BIN_NAME) $(TEST_BIN)

$(LIB_REAL): $(SRC_LIB)
	$(CC) $(CFLAGS) -shared -Wl,-soname,$(LIB_SONAME) -o $@ $^ $(LDFLAGS)

$(LIB_SONAME): $(LIB_REAL)
	ln -sf $(LIB_REAL) $(LIB_SONAME)

$(LIB_LINK): $(LIB_SONAME)
	ln -sf $(LIB_SONAME) $(LIB_LINK)

$(BIN_NAME): $(SRC_BIN) $(LIB_REAL)
	$(CC) $(CFLAGS) -o $@ $(SRC_BIN) -L. -lalttest $(LDFLAGS)

$(TEST_BIN): src/test.c $(LIB_REAL)
	$(CC) $(CFLAGS) -o $@ src/test.c -L. -lalttest $(LDFLAGS)

clean:
	rm -f $(LIB_REAL) $(LIB_SONAME) $(LIB_LINK) $(BIN_NAME) $(TEST_BIN)


PREFIX ?= /usr/local
LIBDIR = $(PREFIX)/lib64
BINDIR = $(PREFIX)/bin

install: $(LIB_REAL) $(LIB_SONAME) $(LIB_LINK) $(BIN_NAME)
	install -D $(LIB_REAL) $(DESTDIR)$(LIBDIR)/$(LIB_REAL)
	install -D $(LIB_SONAME) $(DESTDIR)$(LIBDIR)/$(LIB_SONAME)
	install -D $(LIB_LINK) $(DESTDIR)$(LIBDIR)/$(LIB_LINK)
	install -D $(BIN_NAME) $(DESTDIR)$(BINDIR)/$(BIN_NAME)
	ldconfig $(LIBDIR)

uninstall:
	rm -f $(DESTDIR)$(LIBDIR)/$(LIB_REAL) \
	      $(DESTDIR)$(LIBDIR)/$(LIB_SONAME) \
	      $(DESTDIR)$(LIBDIR)/$(LIB_LINK) \
	      $(DESTDIR)$(BINDIR)/$(BIN_NAME)
	ldconfig $(LIBDIR)
