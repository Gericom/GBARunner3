#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
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

# These set the information text in the nds file
GAME_TITLE     := GBARunner 3
GAME_SUBTITLE1 := By Gericom
GAME_SUBTITLE2 :=

include $(DEVKITARM)/ds_rules

.PHONY: checkcorearm9 checkarm7 checkarm9 clean

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all: checkcorearm9 checkarm7 checkarm9 GBARunner3.nds

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
GBARunner3.nds	: $(NITRO_FILES) ../core/arm7/GBARunner3.elf arm9/GBARunner3.elf
	ndstool	-c GBARunner3.nds -7 ../core/arm7/GBARunner3.elf -9 arm9/GBARunner3.elf \
	-b $(GAME_ICON) "$(GAME_TITLE);$(GAME_SUBTITLE1);$(GAME_SUBTITLE2)" \
	$(_ADDFILES)
ifdef GBAR3_ISNITRO
	dlditool FatInNitroFS.dldi GBARunner3.nds
endif

#---------------------------------------------------------------------------------
../core/arm7/GBARunner3.elf:
	$(MAKE) -C ../core/arm7
	
#---------------------------------------------------------------------------------
arm9/GBARunner3.elf:
	$(MAKE) -C arm9

#---------------------------------------------------------------------------------
clean:
	$(MAKE) -C ../core/arm9 clean
	$(MAKE) -C arm9 clean
	$(MAKE) -C ../core/arm7 clean
	rm -f GBARunner3.nds GBARunner3.arm7 GBARunner3.arm9
	rm -f arm9/data/gbarunner9.bin