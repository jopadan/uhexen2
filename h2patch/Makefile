
UHEXEN2_TOP:=..
LIBS_DIR:=$(UHEXEN2_TOP)/libs

CC ?= gcc
LINKER := $(CC)

#CPUFLAGS= -march=i486
#CPUFLAGS= -march=i586
CPUFLAGS=
CFLAGS  = $(CPUFLAGS) -O3 -Wall -Wshadow
LDFLAGS =

XDELTA_DIR:= $(LIBS_DIR)/xdelta3
XDFLAGS:= -fno-builtin \
	  -DXD3_DEBUG=0 -DNDEBUG=1 \
	  -DXD3_STDIO=1
# when XD3_USE_LARGEFILE64 is not defined, xdelta3 defaults
# to 64 bit xoff_t.
# define XD3_USE_LARGEFILE64 as 0 or 1.
XDFLAGS+= -DXD3_USE_LARGEFILE64=1
XDFLAGS+= -I$(XDELTA_DIR) -I$(LIBS_DIR)/common
XDFLAGS+= $(CFLAGS)

MD5DIR:= $(LIBS_DIR)/md5
MD5FLAGS:= -I$(MD5DIR) -I$(LIBS_DIR)/common
MD5FLAGS+= $(CFLAGS)

CFLAGS  += -I. -I$(XDELTA_DIR) -I$(MD5DIR) -I$(LIBS_DIR)/common

TARGETS:= h2patch

all: $(TARGETS)

MD5DEPS:= md5.h q_endian.h
XD3DEPS:= xdelta3-blkcache.h xdelta3-decode.h xdelta3.h \
	xdelta3-list.h xdelta3-main.h

OBJECTS:= md5.o xdelta3.o qsnprint.o h2patch3.o

MD5DEPS:= md5.c md5.h
MD5DEPS:= $(patsubst %,$(MD5DIR)/%,$(MD5DEPS))

XD3DEPS:= xdelta3-decode.h xdelta3-list.h xdelta3-main.h xdelta3-blkcache.h \
	xdelta3.c xdelta3.h
XD3DEPS:= $(patsubst %,$(XDELTA_DIR)/%,$(XD3DEPS))

# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<
%.o: $(LIBS_DIR)/common/%.c
	$(CC) -c $(CFLAGS) -o $@ $<

xdelta3.o: $(XD3DEPS)
	$(CC) $(XDFLAGS) -c $(XDELTA_DIR)/xdelta3.c -o xdelta3.o
md5.o: $(MD5DEPS)
	$(CC) $(MD5FLAGS) -c $(MD5DIR)/md5.c -o md5.o

h2patch: $(OBJECTS)
	$(LINKER) $(OBJECTS) -o h2patch $(LDFLAGS)

clean:
	rm -f *.o *.res
cleaner: clean
	rm -f $(TARGETS)
