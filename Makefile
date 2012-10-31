CC=gcc

CFLAGS=-Wall -Wextra -ggdb -std=gnu99 -Wno-pointer-sign 
LT = libtool $(SILENTLIB) --tag=CC
LDFLAGS = -lz -lvorbisfile
LDFLAGS += -lcrypto
LD = $(CC)
SILENT := @

OBJS = mini.o
LIBDIR = ../../lib
LIB = despotify

CFLAGS += -I$(LIBDIR)

OBJS += libao.o
LDFLAGS += -lpthread -lao



all: mini

# These are the files we depgen for. :-)
CFILES = $(OBJS:.o=.c)
include ../depgen.mk

mini: $(OBJS) $(LIB)
	@echo LD $@
	$(SILENT)$(LT) --mode=link $(CC) -o $@ $(CFLAGS) $(LDFLAGS) $(OBJS) $(LIB)

clean:
	$(LT) --mode=clean rm -f mini
	rm -f $(OBJS) Makefile.dep

install: simple
	@echo "Copying mini binary to $(INSTALL_PREFIX)/bin/${package}-mini"
	install -d $(INSTALL_PREFIX)/bin/
	$(LT) --mode=install install mini $(INSTALL_PREFIX)/bin/${package}-mini

uninstall:
	@echo "Removing mini..."
	rm -f $(INSTALL_PREFIX)/bin/${package}-mini
