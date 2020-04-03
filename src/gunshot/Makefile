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
	GunShot.cpp

# --------------------------------------------------------------
# Do some magic

include ../../dpf/Makefile.plugins.mk

# --------------------------------------------------------------
# Set include paths

# --------------------------------------------------------------
# Enable all possible plugin types

ifneq ($(MACOS_OR_WINDOWS),true)
TARGETS += ladspa
TARGETS += dssi
endif

TARGETS += lv2_dsp
TARGETS += vst

all: $(TARGETS)

# --------------------------------------------------------------
