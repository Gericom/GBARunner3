# SPDX-License-Identifier: CC0-1.0
#
# SPDX-FileContributor: Antonio Niño Díaz, 2023

export BLOCKSDS			?= /opt/blocksds/core
export BLOCKSDSEXT		?= /opt/blocksds/external

export WONDERFUL_TOOLCHAIN	?= /opt/wonderful
ARM_NONE_EABI_PATH	?= $(WONDERFUL_TOOLCHAIN)/toolchain/gcc-arm-none-eabi/bin/

# Defines passed to all files
# ---------------------------

DEFINES		:= -DLIBTWL_ARM7

# Libraries
# ---------

LIBTWL_DIR	:= $(CURDIR)/../../libs/libtwl
LIBS		:= -lnds7 -ltwl7
LIBDIRS		:= $(BLOCKSDS)/libs/libnds \
			$(LIBTWL_DIR)/libtwl7 $(LIBTWL_DIR)/common $(LIBTWL_DIR)

# Build artifacts
# -----------------

NAME		:= arm7
BUILDDIR	:= build/$(NAME)
ELF			:= build/$(NAME).elf
DUMP		:= build/$(NAME).dump
MAP			:= build/$(NAME).map

# Source code paths
# -----------------

SOURCEDIRS	:= source
INCLUDEDIRS	:= source ../common $(BUILDDIR)
BINDIRS		:=

# Tools
# -----

PREFIX		:= $(ARM_NONE_EABI_PATH)arm-none-eabi-
CC			:= $(PREFIX)gcc
CXX			:= $(PREFIX)g++
LD			:= $(PREFIX)gcc
OBJDUMP		:= $(PREFIX)objdump
MKDIR		:= mkdir
RM			:= rm -rf

# Verbose flag
# ------------

ifeq ($(VERBOSE),1)
V		:=
else
V		:= @
endif

# Source files
# ------------

ifneq ($(BINDIRS),)
    SOURCES_BIN	:= $(shell find -L $(BINDIRS) -name "*.bin")
    INCLUDEDIRS	+= $(addprefix $(BUILDDIR)/,$(BINDIRS))
endif

SOURCES_S	:= $(shell find -L $(SOURCEDIRS) -name "*.s")
SOURCES_C	:= $(shell find -L $(SOURCEDIRS) -name "*.c")
SOURCES_CPP	:= $(shell find -L $(SOURCEDIRS) -name "*.cpp")

# Compiler and linker flags
# -------------------------

ARCH		:= -mthumb -mcpu=arm7tdmi

SPECS		:= gbarunner7.specs

WARNFLAGS	:= -Wall

ifeq ($(SOURCES_CPP),)
	LIBS	+= -lc
else
	LIBS	+= -lstdc++ -lc
endif

INCLUDEFLAGS	:= $(foreach path,$(INCLUDEDIRS),-I$(path)) \
		   $(foreach path,$(LIBDIRS),-I$(path)/include)

LIBDIRSFLAGS	:= $(foreach path,$(LIBDIRS),-L$(path)/lib)

ASFLAGS		+= -x assembler-with-cpp $(DEFINES) $(INCLUDEFLAGS) \
		   $(ARCH) -ffunction-sections -fdata-sections \
		   -specs=$(SPECS)

CFLAGS		+= -std=gnu17 $(WARNFLAGS) $(DEFINES) $(INCLUDEFLAGS) \
		   $(ARCH) -O2 -ffunction-sections -fdata-sections \
		   -specs=$(SPECS)

CXXFLAGS	+= -std=gnu++17 $(WARNFLAGS) $(DEFINES) $(INCLUDEFLAGS) \
		   $(ARCH) -O2 -ffunction-sections -fdata-sections \
		   -fno-exceptions -fno-rtti \
		   -specs=$(SPECS)

LDFLAGS		:= $(ARCH) $(LIBDIRSFLAGS) -Wl,-Map,$(MAP) $(DEFINES) \
		   -Wl,--start-group $(LIBS) -Wl,--end-group -specs=$(SPECS)

# Intermediate build files
# ------------------------

OBJS_ASSETS	:= $(addsuffix .o,$(addprefix $(BUILDDIR)/,$(SOURCES_BIN)))

HEADERS_ASSETS	:= $(patsubst %.bin,%_bin.h,$(addprefix $(BUILDDIR)/,$(SOURCES_BIN)))

OBJS_SOURCES	:= $(addsuffix .o,$(addprefix $(BUILDDIR)/,$(SOURCES_S))) \
		   $(addsuffix .o,$(addprefix $(BUILDDIR)/,$(SOURCES_C))) \
		   $(addsuffix .o,$(addprefix $(BUILDDIR)/,$(SOURCES_CPP)))

OBJS		:= $(OBJS_ASSETS) $(OBJS_SOURCES)

DEPS		:= $(OBJS:.o=.d)

# Targets
# -------

.PHONY: all clean dump

all: $(ELF)

$(ELF): $(OBJS)
	@echo "  LD.7    $@"
	$(V)$(LD) -o $@ $(OBJS) $(LDFLAGS)

$(DUMP): $(ELF)
	@echo "  OBJDUMP.7 $@"
	$(V)$(OBJDUMP) -h -C -S $< > $@

dump: $(DUMP)

clean:
	@echo "  CLEAN.7"
	$(V)$(RM) $(ELF) $(DUMP) $(MAP) $(BUILDDIR)

# Rules
# -----

$(BUILDDIR)/%.s.o : %.s
	@echo "  AS.7    $<"
	@$(MKDIR) -p $(@D)
	$(V)$(CC) $(ASFLAGS) -MMD -MP -c -o $@ $<

$(BUILDDIR)/%.c.o : %.c
	@echo "  CC.7    $<"
	@$(MKDIR) -p $(@D)
	$(V)$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

$(BUILDDIR)/%.arm.c.o : %.arm.c
	@echo "  CC.7    $<"
	@$(MKDIR) -p $(@D)
	$(V)$(CC) $(CFLAGS) -MMD -MP -marm -mlong-calls -c -o $@ $<

$(BUILDDIR)/%.cpp.o : %.cpp
	@echo "  CXX.7   $<"
	@$(MKDIR) -p $(@D)
	$(V)$(CXX) $(CXXFLAGS) -MMD -MP -c -o $@ $<

$(BUILDDIR)/%.arm.cpp.o : %.arm.cpp
	@echo "  CXX.7   $<"
	@$(MKDIR) -p $(@D)
	$(V)$(CXX) $(CXXFLAGS) -MMD -MP -marm -mlong-calls -c -o $@ $<

$(BUILDDIR)/%.bin.o $(BUILDDIR)/%_bin.h : %.bin
	@echo "  BIN2C.7 $<"
	@$(MKDIR) -p $(@D)
	$(V)$(BLOCKSDS)/tools/bin2c/bin2c $< $(@D)
	$(V)$(CC) $(CFLAGS) -MMD -MP -c -o $(BUILDDIR)/$*.bin.o $(BUILDDIR)/$*_bin.c

# All assets must be built before the source code
# -----------------------------------------------

$(SOURCES_S) $(SOURCES_C) $(SOURCES_CPP): $(HEADERS_ASSETS)

# Include dependency files if they exist
# --------------------------------------

-include $(DEPS)
