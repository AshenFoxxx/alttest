CC      = gcc
CFLAGS  = -fPIC -Wall -Wextra -std=c99 -O2 -Iinclude
LDFLAGS = -lcurl -ljansson -lrpm -lrpmio    

LIB_MAJOR = 1
LIB_VERSION = 1.0.0

LIB_REAL  = libalttest.so.$(LIB_VERSION)
LIB_SONAME = libalttest.so.$(LIB_MAJOR)
LIB_LINK  = libalttest.so

BIN_NAME  = alttest
TEST_BIN  = tests  

SRC_LIB  = src/lib.c
SRC_BIN  = src/main.c
SRC_TEST = src/test.c

all: $(LIB_REAL) $(LIB_SONAME) $(LIB_LINK) $(BIN_NAME) $(TEST_BIN)

$(LIB_REAL): $(SRC_LIB)
	$(CC) $(CFLAGS) -shared -Wl,-soname,$(LIB_SONAME) -o $@ $^ $(LDFLAGS)

$(LIB_SONAME): $(LIB_REAL)
	ln -sf $(LIB_REAL) $@

$(LIB_LINK): $(LIB_SONAME)
	ln -sf $(LIB_SONAME) $@

$(BIN_NAME): $(SRC_BIN) $(LIB_LINK)
	$(CC) $(CFLAGS) -o $@ $(SRC_BIN) -L. -lalttest $(LDFLAGS) -Wl,-rpath,'$$ORIGIN'

$(TEST_BIN): $(SRC_TEST) $(LIB_LINK)
	$(CC) $(CFLAGS) -o $@ $(SRC_TEST) -L. -lalttest $(LDFLAGS) -Wl,-rpath,'$$ORIGIN'

run-tests: $(TEST_BIN)
	LD_LIBRARY_PATH=. ./$(TEST_BIN)

check: run-tests

PREFIX ?= /usr/local
LIBDIR = $(PREFIX)/lib64
BINDIR = $(PREFIX)/bin

install: $(LIB_REAL) $(LIB_SONAME) $(LIB_LINK) $(BIN_NAME)
	install -D -m755 $(LIB_REAL)     $(DESTDIR)$(LIBDIR)/$(LIB_REAL)
	ln -sf $(LIB_REAL)              $(DESTDIR)$(LIBDIR)/$(LIB_SONAME)
	ln -sf $(LIB_SONAME)            $(DESTDIR)$(LIBDIR)/$(LIB_LINK)
	install -D -m755 $(BIN_NAME)    $(DESTDIR)$(BINDIR)/$(BIN_NAME)

uninstall:
	rm -f $(DESTDIR)$(LIBDIR)/$(LIB_REAL)
	rm -f $(DESTDIR)$(LIBDIR)/$(LIB_SONAME) $(DESTDIR)$(LIBDIR)/$(LIB_LINK)
	rm -f $(DESTDIR)$(BINDIR)/$(BIN_NAME)
	/sbin/ldconfig

clean:
	rm -f $(LIB_REAL) $(LIB_SONAME) $(LIB_LINK) $(BIN_NAME) $(TEST_BIN)

.PHONY: all run-tests check install uninstall clean
