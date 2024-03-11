#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules

#---------------------------------------------------------------------------------
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
# DATA is a list of directories containing binary files
# all directories are relative to this makefile
#---------------------------------------------------------------------------------
BUILD		:=	build
SOURCES		:=	source \
				source/core \
				source/core/heap \
				source/logger \
				source/tests \
				source/tests/MemoryEmulator \
				source/tests/MemoryEmulator/Arm \
				source/tests/MemoryEmulator/Thumb \
				source/tests/VirtualMachine \
				../../core/arm9/source/Emulator \
				../../core/arm9/source/MemoryEmulator/Arm \
				../../core/arm9/source/MemoryEmulator/Thumb \
				../../core/arm9/source/Peripherals \
				../../core/arm9/source/Peripherals/Graphics \
				../../core/arm9/source/Peripherals/Interrupts \
				../../core/arm9/source/VirtualMachine \
				../../libs/googlemock/src \
				../../libs/googletest/src \
				../../libs/mini-printf
INCLUDES	:=	include \
				source \
				../../libs/googlemock \
				../../libs/googletest \
				../../core/arm9/source \
				../../libs/googlemock/include \
				../../libs/googletest/include \
				../../libs/mini-printf \
				../../core/common
DATA		:=  


#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-marm -mthumb-interwork -DARM9 -DLIBTWL_ARM9 -DGBAR3_TEST

CFLAGS	:=	-g -Wall -O2\
 			-march=armv5te -mtune=arm946e-s -fomit-frame-pointer\
			-ffunction-sections -fdata-sections\
			-ffast-math \
			$(ARCH)

CFLAGS	+=	$(INCLUDE)
CXXFLAGS	:=	$(CFLAGS) -std=gnu++23 -Wno-volatile -fno-rtti -fno-exceptions -fno-threadsafe-statics

ASFLAGS	:=	-g $(ARCH) $(INCLUDE) -march=armv5te -mtune=arm946e-s

LDFLAGS	=	-specs=../test9.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map),--gc-sections,--use-blx,--no-warn-rwx-segments -ffunction-sections -fdata-sections

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS	:=	-lnds9
 
#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:=	$(LIBNDS) $(CURDIR)/../../libs/libtwl/libtwl9 $(CURDIR)/../../libs/libtwl/common
 
#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------
 
export ARM9ELF	:=	$(CURDIR)/$(TARGET).elf
export DEPSDIR := $(CURDIR)/$(BUILD)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir)) \
					$(CURDIR)/../../core/arm9/source/ \
					$(CURDIR)/../../core/arm9/source/MemoryEmulator/
 
CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

SFILES += DtcmStack.s MemCopy.s MemoryLoadStoreRemapTable.s
CPPFILES += PopCountTable.cpp
 
#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
					$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
 
export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)
 
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)
 
.PHONY: $(BUILD) clean
 
#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
 
#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) *.elf *.nds* *.bin 
 
 
#---------------------------------------------------------------------------------
else
 
#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(ARM9ELF)	:	$(OFILES)
	@echo linking $(notdir $@)
	@$(LD)  $(LDFLAGS) $(OFILES) $(LIBPATHS) $(LIBS) -o $@

#---------------------------------------------------------------------------------
# you need a rule like this for each extension you use as binary data 
#---------------------------------------------------------------------------------
%.bin.o	:	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

-include $(DEPSDIR)/*.d
 
#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
