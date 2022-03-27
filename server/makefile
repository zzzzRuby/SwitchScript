#
#             LUFA Library
#     Copyright (C) Dean Camera, 2014.
#
#  dean [at] fourwalledcubicle [dot] com
#           www.lufa-lib.org
#
# --------------------------------------
#         LUFA Project Makefile.
# --------------------------------------

# Run "make help" for target help.

MCU            = atmega32u4
ARCH           = AVR8
F_CPU          = 16000000
F_USB          = $(F_CPU)
OPTIMIZATION   = s
TARGET         = SwitchScriptServer
SRC            = src/SwitchScriptServer.c src/Descriptors.c ../vm/src/vm.c ../vm/src/avr/vm_millis.c $(LUFA_SRC_USB)
LUFA_PATH      = ../third_party/lufa/LUFA
CC_FLAGS       = -DUSE_LUFA_CONFIG_HEADER -Iinclude/ -I../vm/include/ -fstack-usage
LD_FLAGS       =
OBJDIR         = build
SUDO           =

all:

include $(LUFA_PATH)/Build/lufa_core.mk
include $(LUFA_PATH)/Build/lufa_sources.mk
include $(LUFA_PATH)/Build/lufa_build.mk