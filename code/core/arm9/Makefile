#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

%.size.o: %.size.cpp
	$(SILENTMSG) $(notdir $<)
	$(SILENTCMD)$(CXX) -MMD -MP -MF $(DEPSDIR)/$*.size.d $(CXXFLAGS) -mthumb -Os -c $< -o $@ $(ERROR_FILTER)

include $(DEVKITARM)/ds_rules

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# DATA is a list of directories containing data files
# INCLUDES is a list of directories containing header files
# SPECS is the directory containing the important build and link files
#---------------------------------------------------------------------------------
export TARGET		:=	$(shell basename $(CURDIR))
BUILD		:=	build
SOURCES		:=	source \
				../common \
				source/Application \
				source/Application/Settings \
				source/Application/Settings/Enums \
				source/Application/Settings/Json \
				source/Core \
				source/Core/Heap \
				source/Emulator \
				source/Fat \
				source/JitPatcher \
				source/Logger \
				source/MemoryEmulator \
				source/MemoryEmulator/Arm \
				source/MemoryEmulator/Thumb \
				source/Patches \
				source/Peripherals \
				source/Peripherals/Graphics \
				source/Peripherals/Interrupts \
				source/Peripherals/Sound \
				source/Save \
				source/SdCache \
				source/VirtualMachine \
				../../libs/mini-printf
DATA		:=	data
INCLUDES	:=	include source ../common ../../libs/mini-printf


#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-marm -mthumb-interwork -march=armv5te -mtune=arm946e-s \
			-DLIBTWL_ARM9 -DARM9 -DGBAR3_HICODE_CACHE_MAPPING

CFLAGS	:=	-g -Wall -O2\
			 -fomit-frame-pointer\
			-ffunction-sections -fdata-sections\
			-ffast-math \
			-Werror=return-type \
			$(ARCH)

CFLAGS	+=	$(INCLUDE)

CXXFLAGS	:= $(CFLAGS) -std=gnu++23 -Wno-volatile -fno-rtti -fno-exceptions -fno-threadsafe-statics\
				-Wsuggest-override -Werror=suggest-override

ASFLAGS	:=	-g $(ARCH) $(INCLUDE)
LDFLAGS	=	-T ../gbarunner9.ld -g $(ARCH) -Wl,-Map,$(notdir $*.map),--use-blx,--no-warn-rwx-segments,--gc-sections -ffunction-sections -fdata-sections

LIBS	:= -ltwl9 -lnds9

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:=	$(LIBNDS)  $(CURDIR)/../../libs/libtwl/libtwl9 $(CURDIR)/../../libs/libtwl/common
 
  
#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------
 
export OUTPUT	:=	$(CURDIR)/$(TARGET)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
			$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

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

.PHONY: $(BUILD) clean all
 
#---------------------------------------------------------------------------------
all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
 
#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).bin $(TARGET).elf
 
 
#---------------------------------------------------------------------------------
else
 
DEPENDS	:=	$(OFILES:.o=.d)
 
#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT).bin	:	$(OUTPUT).elf
$(OUTPUT).elf	:	$(OFILES)


#---------------------------------------------------------------------------------
%.bin: %.elf
	@$(OBJCOPY) -O binary $< $@
	@echo built ... $(notdir $@)
 

-include $(DEPENDS)

 
#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
