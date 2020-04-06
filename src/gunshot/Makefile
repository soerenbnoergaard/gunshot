#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
#

# --------------------------------------------------------------
# Project name, used for binaries

NAME = gunshot

# --------------------------------------------------------------
# Files to build

FILES_DSP = \
	GunShot.cpp \
	utils.cpp \
	$(wildcard ../../fftconvolver/*.cpp) \
	../../base64/base64.c \
	$(wildcard ../../libsamplerate/src/*.c)

FILES_UI  = \
	GunShotUI.cpp \
	utils.cpp \
	../../base64/base64.c

FONT_OBJECTS = ../../dejavu-fonts/DejaVuSans.o

# --------------------------------------------------------------
# Do some magic

include ../../dpf/Makefile.plugins.mk

# --------------------------------------------------------------
# Set include paths
BUILD_C_FLAGS += -DPACKAGE='"libsamplerate"' -DVERSION='"0.1.9"' -DCPU_CLIPS_POSITIVE=0 -DCPU_CLIPS_NEGATIVE=0
BUILD_CXX_FLAGS += -I ../../ -I ../../libsamplerate/src
LINK_FLAGS += $(FONT_OBJECTS)

# --------------------------------------------------------------
# Enable all possible plugin types

ifneq ($(MACOS_OR_WINDOWS),true)
TARGETS += ladspa
TARGETS += dssi
endif

TARGETS += lv2_dsp
TARGETS += vst

all: $(FONT_OBJECTS) $(TARGETS)
	cp ../../bin/gunshot-vst.so ~/.vst/soerenbnoergaard

# --------------------------------------------------------------

%.o:%.ttf
	$(LD) -r -b binary -o $@ $<

