#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

ifneq (,$(shell command -v python3 2>/dev/null))
PYTHON	:= python3
else ifneq (,$(shell command -v python2 2>/dev/null))
PYTHON	:= python2
else ifneq (,$(shell command -v python 2>/dev/null))
PYTHON	:= python
else
$(error "Python not found in PATH, please install it.")
endif

export TARGET	:=	GBARunner3
export TOPDIR	:=	$(CURDIR)

# specify a directory which contains the nitro filesystem
# this is relative to the Makefile
ifdef GBAR3_ISNITRO
NITRO_FILES	:= nitrofiles
else
NITRO_FILES	:= 
endif

# GAME_ICON is the image used to create the game icon, leave blank to use default rule
GAME_ICON := $(CURDIR)/icon.bmp

# These set the information text in the nds file
GAME_TITLE     := GBARunner3
GAME_SUBTITLE1 := By Gericom
GAME_SUBTITLE2 := ""

include $(DEVKITARM)/ds_rules

.PHONY: checkcorearm9 checkarm7 checkarm9 clean

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all: checkcorearm9 checkarm7 checkarm9 $(TARGET).nds

#---------------------------------------------------------------------------------
checkcorearm9:
	$(MAKE) -C ../core/arm9
	cp ../core/arm9/arm9.bin arm9/data/gbarunner9.bin

#---------------------------------------------------------------------------------
checkarm7:
	$(MAKE) -C ../core/arm7
	
#---------------------------------------------------------------------------------
checkarm9:
	$(MAKE) -C arm9

#---------------------------------------------------------------------------------
ANIMATED ?= ON

$(TARGET).nds	: $(NITRO_FILES) ../core/arm7/$(TARGET).elf arm9/$(TARGET).elf
	ndstool	-c $(TARGET).nds -7 ../core/arm7/$(TARGET).elf -9 arm9/$(TARGET).elf \
	-b $(GAME_ICON) "$(GAME_TITLE);$(GAME_SUBTITLE1);$(GAME_SUBTITLE2)" \
	$(_ADDFILES)
ifeq ($(ANIMATED),ON)
	$(PYTHON) AnimatedBannerPatch.py $@ icon.bin
else
	@echo "Static Icon applied"
endif
ifdef GBAR3_ISNITRO
	dlditool FatInNitroFS.dldi $(TARGET).nds
endif

#---------------------------------------------------------------------------------
../core/arm7/$(TARGET).elf:
	$(MAKE) -C ../core/arm7
	
#---------------------------------------------------------------------------------
arm9/$(TARGET).elf:
	$(MAKE) -C arm9

#---------------------------------------------------------------------------------
clean:
	$(MAKE) -C ../core/arm9 clean
	$(MAKE) -C arm9 clean
	$(MAKE) -C ../core/arm7 clean
	rm -f $(TARGET).nds $(TARGET).arm7 $(TARGET).arm9
	rm -f arm9/data/gbarunner9.bin
