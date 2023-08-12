#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

export TARGET	:=	$(shell basename $(CURDIR))
export TOPDIR	:=	$(CURDIR)

include $(DEVKITARM)/ds_rules

.PHONY: checkbootstrap checktest clean

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all: checkbootstrap checktest

#---------------------------------------------------------------------------------
checkbootstrap:
	$(MAKE) -C bootstrap

#---------------------------------------------------------------------------------
checktest:
	$(MAKE) -C test

#---------------------------------------------------------------------------------
clean:
	$(MAKE) -C bootstrap clean
	$(MAKE) -C test clean
