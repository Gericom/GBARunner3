#---------------------------------------------------------------------------------
# Master Makefile by Vea
#---------------------------------------------------------------------------------
CODE_DIR        := code
BIN_DIR         := $(CODE_DIR)/bootstrap/GBARunner3.nds
OUTPUT_DIR      := out
GBA_OUT_DIR     := $(OUTPUT_DIR)/_gba
BIN_DEST        := $(OUTPUT_DIR)/GBARunner3.nds
CONFIGS_SRC     := configs
CONFIGS_DST     := $(GBA_OUT_DIR)/configs
SOURCE_JSON     := $(CONFIGS_DST)/_GBARunner3.json
CONFIG_JSON     := $(GBA_OUT_DIR)/GBARunner3.json

.PHONY: all clean build organize

all: build organize

build:
	@echo "Building GBARunner3..."
	$(MAKE) -C $(CODE_DIR)

organize:
	@mkdir -p $(GBA_OUT_DIR)
	cp "$(BIN_DIR)" "$(BIN_DEST)"
	cp -r "$(CONFIGS_SRC)" "$(GBA_OUT_DIR)"
	mv "$(SOURCE_JSON)" "$(CONFIG_JSON)"
	@echo ""
	@echo "GBARunner3 built successfully!"
	@echo "Check the /out folder."
	@echo ""
	@echo "GBARunner3 by Gericom."

clean:
	@echo "Cleaning..."
	$(MAKE) -C $(CODE_DIR) clean
	@rm -rf "$(OUTPUT_DIR)"
