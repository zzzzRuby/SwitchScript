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

MCU            = atmega16u2
ARCH           = AVR8
F_CPU          = 16000000
F_USB          = $(F_CPU)
OPTIMIZATION   = s
TARGET         = SwitchScriptServer
SRC            = src/SwitchScriptServer.c src/Descriptors.c src/Arduino/wiring.c $(LUFA_SRC_USB)
LUFA_PATH      = ../third_party/lufa/LUFA
CC_FLAGS       = -DUSE_LUFA_CONFIG_HEADER -Isrc/Config/
LD_FLAGS       =
OBJDIR         = build
SUDO           =
SCRIPT_FILE    = scripts/PMSw&Sh-Offline-Pass-Frame.txt
DFU_PROGRAMMER = dfu-programmer.exe

all:

include $(LUFA_PATH)/Build/lufa_core.mk
include $(LUFA_PATH)/Build/lufa_sources.mk
include $(LUFA_PATH)/Build/lufa_build.mk
include $(LUFA_PATH)/Build/lufa_cppcheck.mk
include $(LUFA_PATH)/Build/lufa_doxygen.mk
include $(LUFA_PATH)/Build/lufa_dfu.mk
include $(LUFA_PATH)/Build/lufa_hid.mk
include $(LUFA_PATH)/Build/lufa_avrdude.mk
include $(LUFA_PATH)/Build/lufa_atprogram.mk

flash: all
	$(SUDO) $(DFU_PROGRAMMER) $(MCU) erase || true
	$(SUDO) $(DFU_PROGRAMMER) $(MCU) flash $(TARGET).hex
	$(SUDO) $(DFU_PROGRAMMER) $(MCU) reset