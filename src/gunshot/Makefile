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
	plugin_state.cpp \
	log.c \
	biquad.c \
	utils.c \
	convolver.cpp \
	$(wildcard ../../fftconvolver/*.cpp) \
	../../base64/base64.c \
	$(wildcard ../../libsamplerate/src/*.c)

FILES_UI  = \
	GunShotUI.cpp \
	plugin_state.cpp \
	log.c \
	utils.c \
	../../base64/base64.c

# --------------------------------------------------------------
# Do some magic

include ../../dpf/Makefile.plugins.mk

# --------------------------------------------------------------
# Set include paths and other flags
BUILD_C_FLAGS += -I ../../dpf/distrho/src -DPACKAGE='"libsamplerate"' -DVERSION='"0.1.9"' -DCPU_CLIPS_POSITIVE=0 -DCPU_CLIPS_NEGATIVE=0 -lm
BUILD_CXX_FLAGS += -I ../../dpf/distrho/src -I ../../ -I ../../libsamplerate/src -lm
LINK_FLAGS += $(FONT_OBJECTS) -pthread -lm

# --------------------------------------------------------------
# Enable all possible plugin types

ifneq ($(MACOS_OR_WINDOWS),true)
TARGETS += ladspa
TARGETS += dssi
endif

TARGETS += lv2_dsp
TARGETS += vst

# Defines for bundling a VST plugin for MACOS
BIN_DIR = ../../bin
DPF_DIR =../../dpf
VST_DIR = $(BIN_DIR)/$(NAME).vst

all: $(TARGETS)
	@#cp ../../bin/gunshot-vst.so ~/.vst/soerenbnoergaard
ifeq ($(MACOS),true)
	cp -r $(DPF_DIR)/utils/plugin.vst/ $(VST_DIR)
	mv $(BIN_DIR)/$(NAME)-vst.dylib $(VST_DIR)/Contents/MacOS/$(NAME)
	rm -f $(VST_DIR)/Contents/MacOS/deleteme
	sed -i -e "s/X-PROJECTNAME-X/$(NAME)/" $(VST_DIR)/Contents/Info.plist
	rm -f $(VST_DIR)/Contents/Info.plist-e
endif

# --------------------------------------------------------------
