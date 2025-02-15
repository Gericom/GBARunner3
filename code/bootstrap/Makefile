# SPDX-License-Identifier: CC0-1.0
#
# SPDX-FileContributor: Antonio Niño Díaz, 2023

BLOCKSDS	?= /opt/blocksds/core
BLOCKSDSEXT	?= /opt/blocksds/external

NAME		:= gbarunner3

GAME_TITLE	:= GBARunner 3
GAME_SUBTITLE	:=
GAME_AUTHOR	:= Gericom
GAME_ICON	:= icon.bmp

# Tools
# -----

MAKE		:= make
RM			:= rm -rf

# Verbose flag
# ------------

ifeq ($(VERBOSE),1)
V			:=
else
V			:= @
endif

# DLDI and internal SD slot of DSi
# --------------------------------

# Root folder of the SD image
SDROOT		:= sdroot
# Name of the generated image it "DSi-1.sd" for no$gba in DSi mode
SDIMAGE		:= image.bin

# Source code paths
# -----------------

# specify a directory which contains the nitro filesystem
# this is relative to the Makefile
ifdef GBAR3_ISNITRO
NITROFSDIR	:= nitrofiles
else
NITROFSDIR	:=
endif

# Directories
# -----------

ARM7COREDIR	:= ../core/arm7
ARM9COREDIR	:= ../core/arm9
ARM9DIR		:= arm9

# Build artfacts
# --------------

ROM			:= $(NAME).nds

.PHONY: all checkcorearm9 checkcorearm7 checkarm9 clean sdimage

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all: checkcorearm9 checkcorearm7 checkarm9 $(ROM)

#---------------------------------------------------------------------------------
checkcorearm9:
	$(V)+$(MAKE) -C $(ARM9COREDIR)
	cp $(ARM9COREDIR)/arm9.bin arm9/data/gbarunner9.bin

#---------------------------------------------------------------------------------
checkcorearm7:
	$(V)+$(MAKE) -C $(ARM7COREDIR)
	
#---------------------------------------------------------------------------------
checkarm9: checkcorearm9
	$(V)+$(MAKE) -C $(ARM9DIR) --no-print-directory

#---------------------------------------------------------------------------------
ifneq ($(strip $(NITROFSDIR)),)
# Additional arguments for ndstool
NDSTOOL_ARGS	:= -d $(NITROFSDIR)

# Make the NDS ROM depend on the filesystem only if it is needed
$(ROM): $(NITROFSDIR)
endif

#---------------------------------------------------------------------------------

# Combine the title strings
ifeq ($(strip $(GAME_SUBTITLE)),)
    GAME_FULL_TITLE := $(GAME_TITLE);$(GAME_AUTHOR)
else
    GAME_FULL_TITLE := $(GAME_TITLE);$(GAME_SUBTITLE);$(GAME_AUTHOR)
endif

#---------------------------------------------------------------------------------
$(ROM) : $(ARM7COREDIR)/build/arm7.elf $(ARM9DIR)/build/arm9.elf
	@echo "  NDSTOOL $@"
	$(V)$(BLOCKSDS)/tools/ndstool/ndstool -c $@ \
		-7 $(ARM7COREDIR)/build/arm7.elf -9 $(ARM9DIR)/build/arm9.elf \
		-b $(GAME_ICON) "$(GAME_FULL_TITLE)" \
		$(NDSTOOL_ARGS)

sdimage:
	@echo "  MKFATIMG $(SDIMAGE) $(SDROOT)"
	$(V)$(BLOCKSDS)/tools/mkfatimg/mkfatimg -t $(SDROOT) $(SDIMAGE)

#---------------------------------------------------------------------------------
$(ARM7COREDIR)/build/arm7.elf: checkcorearm7
	
#---------------------------------------------------------------------------------
$(ARM9DIR)/build/arm9.elf: checkarm9

#---------------------------------------------------------------------------------
clean:
	$(V)+$(MAKE) -C $(ARM9COREDIR) clean
	$(V)+$(MAKE) -C $(ARM7COREDIR) clean
	$(V)+$(MAKE) -C $(ARM9DIR) clean
	$(V)$(RM) $(ROM) $(NAME).arm7 $(NAME).arm9
	rm -f arm9/data/gbarunner9.bin
