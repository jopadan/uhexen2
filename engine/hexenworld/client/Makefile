# GNU Makefile for hexenworld clients using GCC.
# $Id$
#
# Type "make help" to see the valid targets.
#
# It is ESSENTIAL that you run make clean between different
# types of builds or different types of targets.
#
# Unix platforms require SDL !!!
#
# To cross-compile for Win32 on Unix, you must pass the W32BUILD=1
# argument to make. It would be best if you examine the script named
# build_cross_win32.sh for cross compilation.
#
# To cross-compile for Win64 on Unix, you must pass the W64BUILD=1
# argument to make. Also see build_cross_win64.sh for details.
#
# Build Options:
#
# USE_SOUND :	yes  =	Include sound support (default).
#		no   =	Don't include sound support.
#
# USE_SDLAUDIO:	yes  =	Include SDL audio support (default).
# (for Unix)	no   =	Don't include SDL audio support.
#		SDL audio is enabled on all unix-like platforms.
#
# USE_ALSA :	yes  =	Include ALSA sound support (default).
# (for Unix)	no   =	Don't include alsa support.
#		Requires alsa-lib and alsa-kernel modules >= 1.0.1
#		(0.9.8 and 1.0.0 should also work, but isn't supported.)
#		On any platform other than Linux, ALSA support will be
#		automatically be disabled.
#
# USE_OSS :	yes  =	Include OSS audio support (default).
# (for Unix)	no   =	Don't include OSS support.
#		OSS is enabled on Linux and FreeBSD.
#		It is automatically disabled on other platforms.
#		See snd_sys.h for a more complete list.
#
# USE_SUNAUDIO:	yes  =	Include SUN audio support (default).
# (for Unix)	no   =	Don't include SUN audio support.
#		SUN audio is enabled on OpenBSD, NetBSD and SUN.
#		It is automatically disabled on other platforms.
#		See snd_sys.h for a more complete list.
#
# USE_CDAUDIO :	yes  =	Include CDAudio support (default).
#		no   =	Don't include CDAudio support.
#
# USE_SDLCD:	no   =	Don't use SDL cdaudio (default); platform specific
# (for Unix)		cdrom code will be used.  The only problem with
#			SDL_cdrom is that it lacks proper volume controls.
#			See cd_unix.h for the availability of platform
#			specific cdaudio drivers.
#		yes  =	Use SDL for cdaudio instead of platform specific code
#		NOTE :	SDL dropped support for cd audio in version 1.3.0 and
#			later!
#
# USE_MIDI :	yes  =	Include target's MIDI driver if available (default).
#		no   =	Don't include MIDI driver.
#
# WITH_SPLASHES	yes  =	(default) enable startup splash screens
# (for Win32)	no   =	disable startup splash screens
#
# USE_MGL	yes  =	Use SciTech MGL for Win32 software video driver.
#			Defaults to yes for x86. Force-set to no for Win64.
# (for Win32)	no   =	Use the DIB/GDI-only compatibility software video
#			driver, which is the default for Win64.
#
# USE_WINSOCK2	yes  =	Use WinSock2 and link to ws2_32 instead of wsock32
# (for Win32)	no   =	(default) Use WinSock1.1 for compatibility with old
#			Windows 95 machines.
#
# LINK_DIRECTX	no  =	(default) Don't link to the dsound and dinput at compile
# (for Win32)		time. The binaries will dynamically load the necessary
#			dll files and functions at runtime. This ensures our exe
#			to run on ancient windows versions without any directx
#			installation.
#		yes =	link to the directx libraries at compile time
#
# LINK_GL_LIBS	no  =	(default) Don't link to the opengl libraries at compile
#			time. The binaries will dynamically load the necessary
#			libraries and functions at runtime.
#		yes =	link to the opengl libraries at compile time
#
# USE_3DFXGAMMA	yes =	enable 3dfx glide hacks for native hardware gamma for
# (Linux / FreeBSD)	the old Voodoo Graphics and Voodoo2 cards.
#		no  =	disable 3dfx glide gamma hacks (default).
#
# OPT_EXTRA	yes  =  Some extra optimization flags will be added (default)
#		no   =	No extra optimizations will be made
#
# USE_X86_ASM	yes  =  Use x86 assembly on ia32 machines which is faster.
#			Automatically disabled for any other cpu.
#		no   =	Do not use intel assembly even on ix86.
#
# COMPILE_32BITS yes =  Compile as a 32 bit binary. If you are on a 64 bit
#			platform and having problems with 64 bit compiled
#			binaries, set this option to yes. Default: no .
#			If you set this to yes, you need to have the 32 bit
#			versions of the libraries that you link against.
#		no   =	Compile for the native word size of your platform,
#			which is the default option.
#
# The default compiler is gcc
# To build with a different compiler:	make CC=compiler_name [other stuff]
#
# The default x86 assembler is nasm. To use a different nasm-compatible
# assembler, such as yasm	:	make NASM=yasm [other stuff]
#
# If X installation is somewhere other than /usr/X11R6, such as /usr/X11R7,
# specify it on the command line:	make X11BASE=/your/x11/path [other stuff]
#
# To build for the demo version:	make DEMO=1 [other stuff]
#
# if building a debug version :		make DEBUG=1 [other stuff]
#

# Path settings:
# main uhexen2 relative path
UHEXEN2_TOP:=../../..
LIBS_DIR:=$(UHEXEN2_TOP)/libs
# common sources path:
COMMONDIR:=../../h2shared
COMMON_HW:=../shared

# General options (see explanations at the top)
USE_SOUND=yes
USE_ALSA=yes
USE_OSS=yes
USE_SUNAUDIO=yes
USE_SDLAUDIO=yes
USE_MIDI=yes
USE_CDAUDIO=yes
USE_SDLCD=no
WITH_SPLASHES=yes
USE_MGL=yes
USE_WINSOCK2=no
LINK_DIRECTX=no
OPT_EXTRA=yes
LINK_GL_LIBS=no
USE_3DFXGAMMA=no
USE_X86_ASM=yes
COMPILE_32BITS=no
# Enable/disable codecs for streaming music support:
USE_CODEC_WAVE=yes
USE_CODEC_MP3=yes
USE_CODEC_VORBIS=yes
USE_CODEC_TIMIDITY=yes
# wildmidi is not supported
USE_CODEC_WILDMIDI=no

# include the common dirty stuff
include $(UHEXEN2_TOP)/scripts/makefile.inc

# Names of the binaries
HW_BINARY:=hwcl$(exe_ext)
GLHW_BINARY:=glhwcl$(exe_ext)

#############################################################
# Compiler flags
#############################################################

CPUFLAGS:=
# Overrides for the default CPUFLAGS
ifeq ($(MACH_TYPE),x86)
CPUFLAGS:=-march=i586
endif

# Overrides for the default ARCHFLAGS
#ARCHFLAGS:=

ifdef DEBUG
# Debug type flags

CFLAGS := -g -Wall

else
# Release type flags

CFLAGS := $(CPUFLAGS) -O2 -Wall -DNDEBUG -ffast-math -fexpensive-optimizations

ifeq ($(OPT_EXTRA),yes)
ifeq ($(MACH_TYPE),x86)
ALIGN_OPT:= $(call check_gcc,-falign-loops=2 -falign-jumps=2 -falign-functions=2,)
ifeq ($(ALIGN_OPT),)
ALIGN_OPT:= $(call check_gcc,-malign-loops=2 -malign-jumps=2 -malign-functions=2,)
endif
CFLAGS := $(CFLAGS) $(ALIGN_OPT)
endif

ifeq ($(MACH_TYPE),x86_64)
ALIGN_OPT:= $(call check_gcc,-falign-loops=2 -falign-jumps=2 -falign-functions=2,)
ifeq ($(ALIGN_OPT),)
ALIGN_OPT:= $(call check_gcc,-malign-loops=2 -malign-jumps=2 -malign-functions=2,)
endif
CFLAGS := $(CFLAGS) $(ALIGN_OPT)
endif

CFLAGS := $(CFLAGS) -fomit-frame-pointer
endif
#
endif

ifeq ($(COMPILE_32BITS),yes)
CFLAGS := $(CFLAGS) -m32
endif

CFLAGS := $(CFLAGS) $(ARCHFLAGS)
# end of compiler flags
#############################################################


#############################################################
# Other build flags
#############################################################
EXT_FLAGS:= -DH2W

# disable x86 assembly if it is not an x86.
ifneq ($(MACH_TYPE),x86)
ifeq ($(MACH_TYPE),x86_64)
#  but keep the ia32 assembly on x86_64 if
#  compiling as 32 bits for the s/w renderer
ifneq ($(COMPILE_32BITS),yes)
USE_X86_ASM=no
endif
else
USE_X86_ASM=no
endif
endif

# whether to use x86 assembly
ifeq ($(USE_X86_ASM),yes)
EXT_FLAGS:= $(EXT_FLAGS) -DUSE_INTEL_ASM
endif

ifdef DEMO
EXT_FLAGS+= -DDEMOBUILD
endif

ifdef DEBUG
# This activates some extra code in hexen2/hexenworld C source
EXT_FLAGS+= -DDEBUG=1 -DDEBUG_BUILD=1
endif


#############################################################
# OpenGL settings
#############################################################
GL_DEFS := -DGLQUAKE
GLFLAGS :=
ifeq ($(USE_3DFXGAMMA),yes)
# linux 3dfx gamma hack
GL_DEFS+= -DUSE_3DFXGAMMA
endif
ifeq ($(LINK_GL_LIBS),no)
GL_DEFS+= -DGL_DLSYM
else
GLFLAGS+= $(GL_LINK)
endif


#############################################################
# Unix flags/settings
#############################################################
ifeq ($(TARGET_OS),unix)

INCLUDES:= -I$(X11BASE)/include -I. -I$(COMMON_HW) -I$(COMMONDIR)
LDFLAGS := $(LIBSOCKET) -lm

# We rely on SDL:  SDLQUAKE must be defined for all SDL using
# platforms / targets.
EXT_FLAGS+= -DSDLQUAKE

ifeq ($(USE_SOUND),yes)
ifeq ($(USE_ALSA),no)
EXT_FLAGS+= -DNO_ALSA_AUDIO
else
# snd_alsa uses dlopen() & co.
LDFLAGS += -ldl
endif
ifeq ($(USE_OSS),no)
EXT_FLAGS+= -DNO_OSS_AUDIO
endif
ifeq ($(USE_SUNAUDIO),no)
EXT_FLAGS+= -DNO_SUN_AUDIO
endif
ifeq ($(USE_SDLAUDIO),no)
EXT_FLAGS+= -DNO_SDL_AUDIO
endif
endif

ifeq ($(USE_CDAUDIO),yes)
ifeq ($(USE_SDLCD),yes)
EXT_FLAGS+= -DWITH_SDLCD
endif
endif

endif
# End of Unix settings
#############################################################


#############################################################
# Win32 flags/settings
#############################################################
ifeq ($(TARGET_OS),win32)

# timidity not needed, we have a midi driver. if you want it,
# comment out the following
USE_CODEC_TIMIDITY=no
USE_CODEC_WILDMIDI=no

ifeq ($(USE_WINSOCK2),yes)
EXT_FLAGS+= -D_USE_WINSOCK2
LIBWINSOCK=-lws2_32
else
LIBWINSOCK=-lwsock32
endif

CFLAGS  := $(CFLAGS) -DWIN32_LEAN_AND_MEAN
INCLUDES:= -I$(W32STUFF) -I$(W32STUFF)/dxsdk/include -I. -I$(COMMON_HW) -I$(COMMONDIR)
LDFLAGS := $(LIBWINSOCK) -lwinmm -mwindows

ifneq ($(USE_MGL),yes)
MGL_FLAG:=
MGL_INCL:=
MGL_LINK:=
endif

ifeq ($(LINK_DIRECTX),no)
EXT_FLAGS+= -DDX_DLSYM
else
LDFLAGS += -L$(W32STUFF)/dxsdk/x86 -ldsound -ldinput -ldxguid
endif

ifeq ($(WITH_SPLASHES),no)
EXT_FLAGS+= -DNO_SPLASHES
else
# gl version needs linking to comctl32.dll because of that progress
# bar thingy. it is necessary for the InitCommonControls call, only
# when USE_HEXEN2_PALTEX_CODE is defined as 0 in h2config.h
#GLFLAGS+=-lcomctl32
endif

ifeq ($(USE_CODEC_VORBIS),yes)
CODEC_INC := -I$(W32STUFF)/codecs/include
CODEC_LINK:= -L$(W32STUFF)/codecs/x86
endif
ifeq ($(USE_CODEC_MP3),yes)
CODEC_INC := -I$(W32STUFF)/codecs/include
CODEC_LINK:= -L$(W32STUFF)/codecs/x86
endif
INCLUDES += $(CODEC_INC)
LDFLAGS += $(CODEC_LINK)

endif
# End of Win32 settings
#############################################################


#############################################################
# Win64 extra flags/settings
#############################################################
ifeq ($(TARGET_OS),win64)

# timidity not needed, we have a midi driver. if you want it,
# comment out the following
USE_CODEC_TIMIDITY=no
USE_CODEC_WILDMIDI=no

# cant use MGL for Win64
USE_MGL=no
MGL_FLAG:=
MGL_INCL:=
MGL_LINK:=
# use winsock2 for win64
USE_WINSOCK2=yes

ifeq ($(USE_WINSOCK2),yes)
EXT_FLAGS+= -D_USE_WINSOCK2
LIBWINSOCK=-lws2_32
else
LIBWINSOCK=-lwsock32
endif

CFLAGS  := $(CFLAGS) -DWIN32_LEAN_AND_MEAN
INCLUDES:= -I$(W32STUFF) -I$(W32STUFF)/dxsdk_new/include -I. -I$(COMMON_HW) -I$(COMMONDIR)
LDFLAGS := $(LIBWINSOCK) -lwinmm -mwindows

ifeq ($(LINK_DIRECTX),no)
EXT_FLAGS+= -DDX_DLSYM
else
LDFLAGS += -L$(W32STUFF)/dxsdk/x64 -ldsound -ldinput -ldxguid
endif

ifeq ($(WITH_SPLASHES),no)
EXT_FLAGS+= -DNO_SPLASHES
else
# gl version needs linking to comctl32.dll because of that progress
# bar thingy. it is necessary for the InitCommonControls call, only
# when USE_HEXEN2_PALTEX_CODE is defined as 0 in h2config.h
#GLFLAGS+=-lcomctl32
endif

ifeq ($(USE_CODEC_VORBIS),yes)
CODEC_INC := -I$(W32STUFF)/codecs/include
CODEC_LINK:= -L$(W32STUFF)/codecs/x64
endif
ifeq ($(USE_CODEC_MP3),yes)
CODEC_INC := -I$(W32STUFF)/codecs/include
CODEC_LINK:= -L$(W32STUFF)/codecs/x64
endif
INCLUDES += $(CODEC_INC)
LDFLAGS += $(CODEC_LINK)

endif
# End of Win64 settings
#############################################################


#############################################################
# Streaming music settings
#############################################################
ifneq ($(USE_SOUND),yes)
# if no sound, disable them:
USE_CODEC_WAVE=no
USE_CODEC_TIMIDITY=no
USE_CODEC_WILDMIDI=no
USE_CODEC_MP3=no
USE_CODEC_VORBIS=no
endif

ifeq ($(USE_CODEC_WAVE),yes)
EXT_FLAGS+= -DUSE_CODEC_WAVE
endif
ifeq ($(USE_CODEC_VORBIS),yes)
EXT_FLAGS+= -DUSE_CODEC_VORBIS
LDFLAGS+= -lvorbis -lvorbisfile -logg
endif
ifeq ($(USE_CODEC_MP3),yes)
EXT_FLAGS+= -DUSE_CODEC_MP3
LDFLAGS+= -lmad
endif
ifeq ($(USE_CODEC_TIMIDITY),yes)
EXT_FLAGS+= -DUSE_CODEC_TIMIDITY
LDFLAGS+= -L$(LIBS_DIR)/timidity -ltimidity
endif
ifeq ($(USE_CODEC_WILDMIDI),yes)
EXT_FLAGS+= -DUSE_CODEC_WILDMIDI
LDFLAGS+= -lWildMidi
endif

# End of streaming music settings
#############################################################


#############################################################
# any other settings/flags
#############################################################
ifeq ($(COMPILE_32BITS),yes)
LDFLAGS:= $(LDFLAGS) -m32
endif

# End of other settings/flags
#############################################################


# Rules for turning source files into .o files
%.o: %.c
	$(CC) -c $(CFLAGS) $(EXT_FLAGS) $(SDLFLAGS) $(INCLUDES) -o $@ $<
%.o: $(COMMON_HW)/%.c
	$(CC) -c $(CFLAGS) $(EXT_FLAGS) $(SDLFLAGS) $(INCLUDES) -o $@ $<
%.o: $(COMMONDIR)/%.c
	$(CC) -c $(CFLAGS) $(EXT_FLAGS) $(SDLFLAGS) $(INCLUDES) -o $@ $<
%.o: %.asm
	$(NASM) $(NASMFLAGS) -I./ -I$(COMMON_HW)/ -I$(COMMONDIR)/ -o $@ $<
%.o: $(COMMON_HW)/%.asm
	$(NASM) $(NASMFLAGS) -I./ -I$(COMMON_HW)/ -I$(COMMONDIR)/ -o $@ $<
%.o: $(COMMONDIR)/%.asm
	$(NASM) $(NASMFLAGS) -I./ -I$(COMMON_HW)/ -I$(COMMONDIR)/ -o $@ $<
%.res: %.rc
	$(WINDRES) -DWIN32_LEAN_AND_MEAN $(EXT_FLAGS) -I. -I$(COMMON_HW) -I$(COMMONDIR) --output-format=coff -o $@ $<
%.res: $(COMMON_HW)/%.rc
	$(WINDRES) -DWIN32_LEAN_AND_MEAN $(EXT_FLAGS) -I. -I$(COMMON_HW) -I$(COMMONDIR) --output-format=coff -o $@ $<
%.res: $(COMMONDIR)/%.rc
	$(WINDRES) -DWIN32_LEAN_AND_MEAN $(EXT_FLAGS) -I. -I$(COMMON_HW) -I$(COMMONDIR) --output-format=coff -o $@ $<

# Objects

# Intel asm objects
ifeq ($(USE_X86_ASM),yes)
COMMON_ASM= math.o \
	sys_ia32.o

SOFT_ASM = \
	d_draw.o \
	d_draw16.o \
	d_draw16t.o \
	d_parta.o \
	d_partb.o \
	d_polysa.o \
	d_polysa2.o \
	d_polysa3.o \
	d_polysa4.o \
	d_polysa5.o \
	d_scana.o \
	d_spr8.o \
	d_spr8t.o \
	d_spr8t2.o \
	d_varsa.o \
	r_aclipa.o \
	r_aliasa.o \
	r_drawa.o \
	r_edgea.o \
	r_edgeb.o \
	r_varsa.o \
	surf8.o \
	surf16.o

SOUND_ASM = snd_mixa.o

else

SOFT_ASM =
COMMON_ASM =
SOUND_ASM =

endif

# Sound objects
ifneq ($(USE_SOUND),yes)
MUSIC_OBJS:= bgmnull.o
SOUND_ASM :=
EXT_FLAGS += -D_NO_SOUND
SYSOBJ_SND :=
COMOBJ_SND := snd_null.o $(MUSIC_OBJS)
else
MUSIC_OBJS:= bgmusic.o \
	snd_codec.o \
	snd_wave.o \
	snd_vorbis.o \
	snd_mp3.o \
	snd_timidity.o \
	snd_wildmidi.o
COMOBJ_SND := snd_sys.o snd_dma.o snd_mix.o $(SOUND_ASM) snd_mem.o $(MUSIC_OBJS)
ifeq ($(TARGET_OS),win32)
SYSOBJ_SND := snd_win.o
endif
ifeq ($(TARGET_OS),win64)
SYSOBJ_SND := snd_win.o
endif
ifeq ($(TARGET_OS),unix)
SYSOBJ_SND := snd_oss.o snd_alsa.o snd_sun.o snd_sdl.o
endif
# end of Sound objects
endif

# MIDI objects
ifneq ($(USE_MIDI),yes)
SYSOBJ_MIDI:= midi_nul.o
EXT_FLAGS += -D_NO_MIDIDRV
else
ifeq ($(TARGET_OS),win32)
SYSOBJ_MIDI:= midi_win.o mid2strm.o
endif
ifeq ($(TARGET_OS),win64)
SYSOBJ_MIDI:= midi_win.o mid2strm.o
endif
ifeq ($(TARGET_OS),unix)
ifeq ($(HOST_OS),darwin)
SYSOBJ_MIDI:= midi_mac.o
else
SYSOBJ_MIDI:= midi_nul.o
EXT_FLAGS += -D_NO_MIDIDRV
endif
endif
# end of MIDI objects
endif

# CDAudio objects
ifneq ($(USE_CDAUDIO),yes)
SYSOBJ_CDA:= cd_null.o
EXT_FLAGS += -D_NO_CDAUDIO
else
ifeq ($(TARGET_OS),win32)
SYSOBJ_CDA := cd_win.o
endif
ifeq ($(TARGET_OS),win64)
SYSOBJ_CDA := cd_win.o
endif
ifeq ($(TARGET_OS),unix)
SYSOBJ_CDA := cd_sdl.o cd_bsd.o cd_linux.o
endif
# end of CDAudio objects
endif

# Other platform specific object settings
ifeq ($(TARGET_OS),win32)
SYSOBJ_INPUT := in_win.o
SYSOBJ_GL_VID:= gl_vidnt.o
ifeq ($(USE_MGL),yes)
SYSOBJ_SOFT_VID:= vid_mgl4.o
else
SYSOBJ_SOFT_VID:= vid_win.o
endif
SYSOBJ_SYS := sys_win.o win32res.res
endif
ifeq ($(TARGET_OS),win64)
SYSOBJ_INPUT := in_win.o
SYSOBJ_GL_VID:= gl_vidnt.o
SYSOBJ_SOFT_VID:= vid_win.o
SYSOBJ_SYS := sys_win.o win32res.res
endif
ifeq ($(TARGET_OS),unix)
SYSOBJ_INPUT := in_sdl.o
SYSOBJ_GL_VID:= gl_vidsdl.o fx_gamma.o
SYSOBJ_SOFT_VID:= vid_sdl.o
SYSOBJ_SYS := sys_unix.o
endif

# Final list of objects
SOFTOBJS = \
	d_edge.o \
	d_fill.o \
	d_init.o \
	d_modech.o \
	d_part.o \
	d_polyse.o \
	d_scan.o \
	d_sky.o \
	d_sprite.o \
	d_surf.o \
	d_vars.o \
	d_zpoint.o \
	r_aclip.o \
	r_alias.o \
	r_bsp.o \
	r_draw.o \
	r_edge.o \
	r_efrag.o \
	r_light.o \
	r_main.o \
	r_misc.o \
	r_part.o \
	r_sky.o \
	r_sprite.o \
	r_surf.o \
	r_vars.o \
	screen.o \
	$(SYSOBJ_SOFT_VID) \
	draw.o \
	model.o

GLOBJS = \
	gl_refrag.o \
	gl_rlight.o \
	gl_rmain.o \
	gl_rmisc.o \
	gl_ngraph.o \
	r_part.o \
	gl_rsurf.o \
	gl_screen.o \
	gl_warp.o \
	$(SYSOBJ_GL_VID) \
	gl_draw.o \
	gl_mesh.o \
	gl_model.o

COMMONOBJS = \
	$(SYSOBJ_INPUT) \
	$(COMOBJ_SND) \
	$(SYSOBJ_SND) \
	$(SYSOBJ_CDA) \
	$(SYSOBJ_MIDI) \
	huffman.o \
	net_wins.o \
	net_chan.o \
	cl_cam.o \
	cl_demo.o \
	cl_effect.o \
	cl_ents.o \
	cl_input.o \
	cl_main.o \
	cl_parse.o \
	cl_pred.o \
	cl_tent.o \
	cl_cmd.o \
	console.o \
	keys.o \
	menu.o \
	sbar.o \
	skin.o \
	view.o \
	wad.o \
	cmd.o \
	q_endian.o \
	link_ops.o \
	sizebuf.o \
	strlcat.o \
	strlcpy.o \
	msg_io.o \
	common.o \
	debuglog.o \
	quakefs.o \
	info_str.o \
	crc.o \
	cvar.o \
	cfgfile.o \
	pr_strng.o \
	mathlib.o \
	pmove.o \
	pmovetst.o \
	zone.o \
	$(SYSOBJ_SYS)


# Targets
.PHONY: help clean cleaner report

#default: glhw
#all: default

help:
	@echo
	@echo "Valid targets: (read/edit the makefile for several options)"
	@echo
	@echo "* $(MAKE) help        : this help"
	@echo "* $(MAKE) clean       : delete all files produced by the build"
	@echo "* $(MAKE) hw          : hexenworld client, software renderer"
	@echo "* $(MAKE) glhw        : hexenworld client, opengl renderer"
	@echo

hwcl: hw
glhwcl: glhw

# extra rule for snd_timidity.c because of timidity/timidity.h and libtimidity.a
snd_timidity.o: $(COMMONDIR)/snd_timidity.c $(LIBS_DIR)/timidity/libtimidity.a
	$(CC) -c $(CFLAGS) $(EXT_FLAGS) $(SDLFLAGS) $(INCLUDES) -I$(LIBS_DIR) -o $@ $<
$(LIBS_DIR)/timidity/libtimidity.a:
	$(MAKE) -C $(LIBS_DIR)/timidity CC="$(CC)" AR="$(AR)" RANLIB="$(RANLIB)"

hw: EXT_FLAGS+=$(MGL_FLAG)
hw: INCLUDES+=$(MGL_INCL)
hw: $(SOFT_ASM) $(SOFTOBJS) $(COMMON_ASM) $(COMMONOBJS)
	$(LINKER) -o $(HW_BINARY) $(SOFT_ASM) $(SOFTOBJS) $(COMMON_ASM) $(COMMONOBJS) $(LDFLAGS) $(SDLLINK) $(MGL_LINK)

glhw: EXT_FLAGS+= $(GL_DEFS)
glhw: $(GLOBJS) $(COMMON_ASM) $(COMMONOBJS)
	$(LINKER) -o $(GLHW_BINARY) $(GLOBJS) $(COMMON_ASM) $(COMMONOBJS) $(LDFLAGS) $(GLFLAGS) $(SDLLINK)

clean:
	rm -f *.o *.res core

cleaner: clean
	rm -f $(HW_BINARY) $(GLHW_BINARY)

report:
	@echo "Host OS  :" $(HOST_OS)
	@echo "Target OS:" $(TARGET_OS)
	@echo "Machine  :" $(MACH_TYPE)
