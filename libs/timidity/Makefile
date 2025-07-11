# GNU Makefile for building a static timidity library using GCC.
#
# To cross-compile for Win32 on Unix: either pass the W32BUILD=1
# argument to make, or export it.  Also see build_win32.sh script.
# Requires: a mingw or mingw-w64 compiler toolchain.
#
# To cross-compile for Win64 on Unix: either pass the W64BUILD=1
# argument to make, or export it. Also see build_win64.sh script.
# Requires: a mingw-w64 compiler toolchain.
#
# To cross-compile for MacOSX on Unix: either pass the OSXBUILD=1
# argument to make, or export it.  You would also need to pass a
# suitable MACH_TYPE=xxx (ppc, x86, x86_64, or ppc64) argument to
# make. Also see build_cross_osx.sh.
#
# To (cross-)compile for DOS: either pass the DOSBUILD=1 argument
# to make, or export it. Also see build_dos.sh script. Requires: a
# djgpp compiler toolchain.
#
# To build a debug version:	make DEBUG=yes
#

UHEXEN2_TOP:=../..
UHEXEN2_SHARED:=$(UHEXEN2_TOP)/common
LIBS_DIR:=$(UHEXEN2_TOP)/libs
OSLIBS:=$(UHEXEN2_TOP)/oslibs

# include the common dirty stuff
include $(UHEXEN2_TOP)/scripts/makefile.inc

LIBTIMIDITY = libtimidity.a

# compiler flags, customize as needed
ifeq ($(MACH_TYPE),x86)
CPU_X86=-march=i586
endif
# Overrides for the default CPUFLAGS
CPUFLAGS=$(CPU_X86)

CFLAGS += -Wall -DTIMIDITY_BUILD=1 -DTIMIDITY_STATIC=1
#CFLAGS += -DLOOKUP_SINE
CFLAGS += $(CPUFLAGS)
ifndef DEBUG
CFLAGS += -O2 -DNDEBUG=1 -ffast-math
# NOTE: -fomit-frame-pointer is broken with ancient gcc versions!!
CFLAGS += -fomit-frame-pointer
else
CFLAGS += -g
endif
#CFLAGS += -DTIMIDITY_DEBUG
INCLUDES = -I.

ifeq ($(TARGET_OS),darwin)
CPUFLAGS=
# require 10.5 for 64 bit builds
ifeq ($(MACH_TYPE),x86_64)
CFLAGS +=-mmacosx-version-min=10.5
endif
ifeq ($(MACH_TYPE),ppc64)
CFLAGS +=-mmacosx-version-min=10.5
endif
endif

ifeq ($(TARGET_OS),win32)
CFLAGS += -m32
#INCLUDES += -I$(OSLIBS)/windows/misc/include
endif
ifeq ($(TARGET_OS),win64)
CFLAGS += -m64
#INCLUDES += -I$(OSLIBS)/windows/misc/include
endif

ifeq ($(TARGET_OS),morphos)
CFLAGS += -noixemul
endif

ifeq ($(TARGET_OS),aros)
CFLAGS += -fno-common
endif

ifeq ($(TARGET_OS),dos)
# nothing extra is needed
endif

ifeq ($(TARGET_OS),os2)
CFLAGS += -Zmt
LIBTIMIDITY:= timidity.a
# -fomit-frame-pointer/-ffast-math seems to cause trouble
# with EMX at least with the old compilers I tried.
CFLAGS  += -fno-omit-frame-pointer
endif


TARGETS:= $(LIBTIMIDITY)

OBJECTS = common.o \
	instrum.o \
	mix.o \
	output.o \
	playmidi.o \
	readmidi.o \
	resample.o \
	stream.o \
	tables.o \
	timidity.o

# Targets
.PHONY: clean distclean

all: $(TARGETS)

# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<

# object dependencies:
common.o: common.c common.h ospaths.h timidity_internal.h
instrum.o: instrum.c common.h instrum.h resample.h tables.h timidity_internal.h
mix.o: mix.c instrum.h mix.h output.h playmidi.h resample.h tables.h timidity_internal.h
output.o: output.c output.h timidity_internal.h
playmidi.o: playmidi.c instrum.h mix.h output.h playmidi.h tables.h timidity_internal.h
readmidi.o: readmidi.c common.h instrum.h playmidi.h timidity_internal.h
resample.o: resample.c common.h instrum.h playmidi.h resample.h tables.h timidity_internal.h
stream.o: stream.c common.h timidity_internal.h
tables.o: tables.c tables.h timidity_internal.h
timidity.o: timidity.c common.h instrum.h ospaths.h output.h playmidi.h readmidi.h tables.h timidity_internal.h
timidity_internal.h: options.h timi_endian.h timidity.h

# rule for target:
$(LIBTIMIDITY): $(OBJECTS)
	$(AR) cru $@ $^
	-$(RANLIB) $@

clean:
	rm -f *.o libtimidity.a timidity.a timidity.lib

distclean: clean

