#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

export TARGET	:=	$(shell basename $(CURDIR))
export TOPDIR	:=	$(CURDIR)

include $(DEVKITARM)/ds_rules

.PHONY: checklibtwl checkbootstrap checktest clean

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all: checklibtwl checkbootstrap checktest

#---------------------------------------------------------------------------------
checklibtwl:
	$(MAKE) -C libs/libtwl

#---------------------------------------------------------------------------------
checkbootstrap:
	$(MAKE) -C bootstrap

#---------------------------------------------------------------------------------
checktest:
	$(MAKE) -C test

#---------------------------------------------------------------------------------
clean:
	$(MAKE) -C libs/libtwl clean
	$(MAKE) -C bootstrap clean
	$(MAKE) -C test clean
