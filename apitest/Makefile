# Makefile for ApiTest Palm OS Application

APP_NAME = ApiTest
BUILD_DIR = build
PRC_NAME = $(BUILD_DIR)/$(APP_NAME).prc
CREATOR_ID = ApTs

# Source files
SOURCES = $(APP_NAME).c
RESOURCES = $(APP_NAME).rcp

# Get script directory for toolchain
TOOLCHAIN_DIR := $(shell cd .. && pwd)/toolchain

# Check if toolchain is installed
ifeq ($(wildcard $(TOOLCHAIN_DIR)/bin/m68k-palmos-gcc),)
  $(error Toolchain not found! Please run: cd .. && ./setup-toolchain.sh)
endif

# Set GCC environment (required for prc-tools to find libraries)
export GCC_EXEC_PREFIX = $(TOOLCHAIN_DIR)/lib/gcc-lib/

# Compiler and tools
CC = $(TOOLCHAIN_DIR)/bin/m68k-palmos-gcc
CFLAGS = -palmos3.5 -nostdinc -O2 -Wall
PILRC = /opt/homebrew/opt/pilrc/bin/pilrc
BUILD_PRC = $(TOOLCHAIN_DIR)/bin/build-prc
PILOT_XFER = /opt/homebrew/opt/pilot-link/bin/pilot-xfer

# Build targets
OBJS = $(BUILD_DIR)/$(APP_NAME).o
APP_BINARY = $(BUILD_DIR)/$(APP_NAME)
RESOURCE_FILE = $(APP_NAME).ro

.PHONY: all clean install info

all: info

info:
	@echo "╔════════════════════════════════════════════════════╗"
	@echo "║          ApiTest - Palm OS Application             ║"
	@echo "╚════════════════════════════════════════════════════╝"
	@echo ""
	@echo " A simple network API call demo for Palm OS"
	@echo ""
	@echo " Available targets:"
	@echo "  make build     - Build $(PRC_NAME)"
	@echo "  make clean     - Remove build artifacts"
	@echo "  make install   - Install to Palm device via USB"
	@echo "  make rebuild   - Clean and build"
	@echo ""

build: $(PRC_NAME)
	@echo "✓ Built $(PRC_NAME)"
	@ls -lh $(PRC_NAME)

$(PRC_NAME): $(APP_BINARY) $(RESOURCE_FILE)
	@mkdir -p $(BUILD_DIR)
	$(BUILD_PRC) -o $@ \
		-t appl \
		-c $(CREATOR_ID) \
		-n "$(APP_NAME)" \
		$(APP_BINARY) $(BUILD_DIR)/*.bin

$(APP_BINARY): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(OBJS) -o $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(RESOURCE_FILE): $(RESOURCES)
	@mkdir -p $(BUILD_DIR)
	$(PILRC) $(RESOURCES) $(BUILD_DIR)

clean:
	/bin/rm -rf $(BUILD_DIR)
	/bin/rm -f *.grc *~

install: $(PRC_NAME)
	@echo "Installing $(PRC_NAME) to Palm device..."
	$(PILOT_XFER) -i $(PRC_NAME)

rebuild: clean build

