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

# --------------------------------------------------------------
# Do some magic

include ../../dpf/Makefile.plugins.mk

# --------------------------------------------------------------
# Set include paths
BUILD_CXX_FLAGS += -I ../../

# --------------------------------------------------------------
# Enable all possible plugin types

ifneq ($(MACOS_OR_WINDOWS),true)
TARGETS += ladspa
TARGETS += dssi
endif

TARGETS += lv2_dsp
TARGETS += vst

all: $(TARGETS)
	cp ../../bin/gunshot-vst.so ~/.vst/soerenbnoergaard

# --------------------------------------------------------------
