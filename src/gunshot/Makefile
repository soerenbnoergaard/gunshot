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
	../../base64/base64.c

FILES_UI  = \
	GunShotUI.cpp \
	utils.cpp \
	../../base64/base64.c

FONT_OBJ = ../../dejavu-fonts/DejaVuSans.o

# --------------------------------------------------------------
# Do some magic

include ../../dpf/Makefile.plugins.mk

# --------------------------------------------------------------
# Set include paths
BUILD_CXX_FLAGS += -I ../../ -I ../../nativefiledialog/src/include/
LINK_FLAGS += ../../nativefiledialog/build/lib/Release/x64/libnfd.a
LINK_FLAGS += $(shell pkg-config --cflags --libs gtk+-3.0)
LINK_FLAGS += $(FONT_OBJ)

# --------------------------------------------------------------
# Enable all possible plugin types

ifneq ($(MACOS_OR_WINDOWS),true)
TARGETS += ladspa
TARGETS += dssi
endif

TARGETS += lv2_dsp
TARGETS += vst

all: $(FONT_OBJ) $(TARGETS)
	cp ../../bin/gunshot-vst.so ~/.vst/soerenbnoergaard

# --------------------------------------------------------------

%.o:%.ttf
	ld -r -b binary -o $@ $<
