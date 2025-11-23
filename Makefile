# Makefile for USB Credential Provider
# Supports both MinGW cross-compilation (Linux) and native Windows compilation

# Detect platform
ifeq ($(OS),Windows_NT)
    PLATFORM := windows
    SHELL := cmd
    MKDIR := mkdir
    RM := rmdir /s /q
    SEP := \\
else
    PLATFORM := linux
    SHELL := /bin/bash
    MKDIR := mkdir -p
    RM := rm -rf
    SEP := /
endif

# Compiler settings
ifeq ($(PLATFORM),linux)
    # Cross-compilation with MinGW-w64
    CXX := x86_64-w64-mingw32-g++
    WINDRES := x86_64-w64-mingw32-windres
    DLLTOOL := x86_64-w64-mingw32-dlltool
else
    # Native Windows compilation
    ifdef MSVC
        CXX := cl
        LINK := link
    else
        CXX := g++
    endif
endif

# Directories
SRC_DIR := src
TOOLS_DIR := tools
BUILD_DIR := build
BIN_DIR := $(BUILD_DIR)$(SEP)bin
OBJ_DIR := $(BUILD_DIR)$(SEP)obj
TOOLS_BIN_DIR := $(BIN_DIR)$(SEP)tools

# Output files
DLL_NAME := USBCredProvider.dll
DLL_PATH := $(BIN_DIR)$(SEP)$(DLL_NAME)
TOOL_NAME := list_usb_devices.exe
TOOL_PATH := $(TOOLS_BIN_DIR)$(SEP)$(TOOL_NAME)

# Source files for DLL
DLL_SOURCES := \
	$(SRC_DIR)/dll.cpp \
	$(SRC_DIR)/helpers.cpp \
	$(SRC_DIR)/guid.cpp \
	$(SRC_DIR)/usb_detector.cpp \
	$(SRC_DIR)/config.cpp \
	$(SRC_DIR)/USBCredential.cpp \
	$(SRC_DIR)/USBCredentialProvider.cpp

# Source files for tool
TOOL_SOURCES := \
	$(TOOLS_DIR)/list_usb_devices.cpp \
	$(SRC_DIR)/usb_detector.cpp

# Object files
ifeq ($(PLATFORM),linux)
    DLL_OBJECTS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(DLL_SOURCES))
    TOOL_OBJ_DIR := $(OBJ_DIR)/tools
    TOOL_OBJECTS := $(OBJ_DIR)/tools/list_usb_devices.o $(OBJ_DIR)/usb_detector.o
else
    DLL_OBJECTS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)\\%.obj,$(DLL_SOURCES))
    TOOL_OBJECTS := $(OBJ_DIR)\\tools\\list_usb_devices.obj $(OBJ_DIR)\\usb_detector.obj
endif

# Compiler flags
CXXFLAGS := -std=c++20 -DUNICODE -D_UNICODE -DWIN32 -D_WIN32
INCLUDES := -I$(SRC_DIR)

ifeq ($(PLATFORM),linux)
    CXXFLAGS += -Wall -Wextra -O2
    CXXFLAGS += -I/usr/x86_64-w64-mingw32/include
    CXXFLAGS += -I/usr/share/mingw-w64/include
else ifdef MSVC
    CXXFLAGS := /std:c++20 /EHsc /DUNICODE /D_UNICODE /DWIN32 /D_WIN32 /MD /O2
    INCLUDES := /I$(SRC_DIR)
else
    CXXFLAGS += -Wall -Wextra -O2
endif

# Linker flags and libraries
ifeq ($(PLATFORM),linux)
    DLL_LDFLAGS := -shared -static-libgcc -static-libstdc++
    DLL_LIBS := -ladvapi32 -lole32 -loleaut32 -lcredui -lsetupapi -luuid -lshlwapi -lwinhttp -lcfgmgr32 -lsecur32
    TOOL_LDFLAGS := -municode -static-libgcc -static-libstdc++
    TOOL_LIBS := -lsetupapi -lcfgmgr32
else ifdef MSVC
    DLL_LIBS := advapi32.lib ole32.lib oleaut32.lib credui.lib setupapi.lib uuid.lib shlwapi.lib winhttp.lib cfgmgr32.lib secur32.lib
    TOOL_LIBS := setupapi.lib cfgmgr32.lib
else
    DLL_LDFLAGS := -shared -static-libgcc -static-libstdc++
    DLL_LIBS := -ladvapi32 -lole32 -loleaut32 -lcredui -lsetupapi -luuid -lshlwapi -lwinhttp -lcfgmgr32 -lsecur32
    TOOL_LDFLAGS := -municode -static-libgcc -static-libstdc++
    TOOL_LIBS := -lsetupapi -lcfgmgr32
endif

# DEF file
DEF_FILE := $(SRC_DIR)/USBCredProvider.def

# Colors (Linux only)
ifeq ($(PLATFORM),linux)
    COLOR_RESET := \\033[0m
    COLOR_GREEN := \\033[1;32m
    COLOR_YELLOW := \\033[1;33m
    COLOR_BLUE := \\033[1;34m
    COLOR_CYAN := \\033[1;36m
    COLOR_RED := \\033[1;31m
else
    COLOR_RESET :=
    COLOR_GREEN :=
    COLOR_YELLOW :=
    COLOR_BLUE :=
    COLOR_CYAN :=
    COLOR_RED :=
endif

# Default target
.PHONY: all
all: banner dirs $(DLL_PATH) $(TOOL_PATH) success

# Banner
.PHONY: banner
banner:
ifeq ($(PLATFORM),linux)
	@echo "$(COLOR_CYAN)╔══════════════════════════════════════════════════════════╗$(COLOR_RESET)"
	@echo "$(COLOR_CYAN)║      USB Credential Provider - Compilation               ║$(COLOR_RESET)"
	@echo "$(COLOR_CYAN)╚══════════════════════════════════════════════════════════╝$(COLOR_RESET)"
	@echo ""
	@echo "$(COLOR_YELLOW)Platform:$(COLOR_RESET) Linux (cross-compiling for Windows)"
	@echo "$(COLOR_YELLOW)Compiler:$(COLOR_RESET) $(CXX)"
	@echo ""
else
	@echo ================================================================
	@echo       USB Credential Provider - Compilation
	@echo ================================================================
	@echo.
	@echo Platform: Windows
	@echo Compiler: $(CXX)
	@echo.
endif

# Create directories
.PHONY: dirs
dirs:
ifeq ($(PLATFORM),linux)
	@$(MKDIR) $(BIN_DIR) $(OBJ_DIR) $(TOOLS_BIN_DIR) $(OBJ_DIR)/tools
else
	@if not exist "$(BUILD_DIR)" $(MKDIR) "$(BUILD_DIR)"
	@if not exist "$(BIN_DIR)" $(MKDIR) "$(BIN_DIR)"
	@if not exist "$(OBJ_DIR)" $(MKDIR) "$(OBJ_DIR)"
	@if not exist "$(TOOLS_BIN_DIR)" $(MKDIR) "$(TOOLS_BIN_DIR)"
	@if not exist "$(OBJ_DIR)\\tools" $(MKDIR) "$(OBJ_DIR)\\tools"
endif

# Build DLL (Linux)
ifeq ($(PLATFORM),linux)
$(DLL_PATH): $(DLL_OBJECTS) $(DEF_FILE)
	@echo "$(COLOR_YELLOW)[LINK]$(COLOR_RESET) Creating DLL: $(COLOR_GREEN)$@$(COLOR_RESET)"
	@$(CXX) $(DLL_LDFLAGS) -o $@ $(DLL_OBJECTS) $(DEF_FILE) $(DLL_LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "$(COLOR_BLUE)[CXX]$(COLOR_RESET)  Compiling: $<"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(TOOL_PATH): $(TOOL_OBJECTS)
	@echo "$(COLOR_YELLOW)[LINK]$(COLOR_RESET) Creating tool: $(COLOR_GREEN)$@$(COLOR_RESET)"
	@$(CXX) $(TOOL_LDFLAGS) -o $@ $(TOOL_OBJECTS) $(TOOL_LIBS)

$(OBJ_DIR)/tools/list_usb_devices.o: $(TOOLS_DIR)/list_usb_devices.cpp
	@echo "$(COLOR_BLUE)[CXX]$(COLOR_RESET)  Compiling: $<"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

else
# Build DLL (Windows with GCC/MinGW)
ifndef MSVC
$(DLL_PATH): $(DLL_OBJECTS) $(DEF_FILE)
	@echo [LINK] Creating DLL: $@
	@$(CXX) $(DLL_LDFLAGS) -o $@ $(DLL_OBJECTS) $(DEF_FILE) $(DLL_LIBS)

$(OBJ_DIR)\\%.obj: $(SRC_DIR)\\%.cpp
	@echo [CXX]  Compiling: $<
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(TOOL_PATH): $(TOOL_OBJECTS)
	@echo [LINK] Creating tool: $@
	@$(CXX) $(TOOL_LDFLAGS) -o $@ $(TOOL_OBJECTS) $(TOOL_LIBS)

$(OBJ_DIR)\\tools\\list_usb_devices.obj: $(TOOLS_DIR)\\list_usb_devices.cpp
	@echo [CXX]  Compiling: $<
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
endif
endif

# Success message
.PHONY: success
success:
ifeq ($(PLATFORM),linux)
	@echo ""
	@echo "$(COLOR_GREEN)╔══════════════════════════════════════════════════════════╗$(COLOR_RESET)"
	@echo "$(COLOR_GREEN)║              Compilation réussie !                       ║$(COLOR_RESET)"
	@echo "$(COLOR_GREEN)╚══════════════════════════════════════════════════════════╝$(COLOR_RESET)"
	@echo ""
	@echo "$(COLOR_CYAN)Fichiers générés:$(COLOR_RESET)"
	@echo "  • $(COLOR_GREEN)$(DLL_PATH)$(COLOR_RESET)"
	@echo "  • $(COLOR_GREEN)$(TOOL_PATH)$(COLOR_RESET)"
	@echo ""
else
	@echo.
	@echo ================================================================
	@echo               Compilation reussie !
	@echo ================================================================
	@echo.
	@echo Fichiers generes:
	@echo   - $(DLL_PATH)
	@echo   - $(TOOL_PATH)
	@echo.
endif

# Clean
.PHONY: clean
clean:
ifeq ($(PLATFORM),linux)
	@echo "$(COLOR_YELLOW)[CLEAN]$(COLOR_RESET) Removing build directory..."
	@$(RM) $(BUILD_DIR)
	@echo "$(COLOR_GREEN)✓ Clean complete$(COLOR_RESET)"
else
	@echo [CLEAN] Removing build directory...
	@if exist "$(BUILD_DIR)" $(RM) "$(BUILD_DIR)"
	@echo Clean complete
endif

# Check MinGW (Linux only)
.PHONY: check-mingw
check-mingw:
ifeq ($(PLATFORM),linux)
	@echo "$(COLOR_YELLOW)[CHECK]$(COLOR_RESET) Checking MinGW-w64 installation..."
	@which $(CXX) > /dev/null 2>&1 || (echo "$(COLOR_RED)ERROR: MinGW-w64 not found!$(COLOR_RESET)" && \
		echo "Install with: sudo apt-get install mingw-w64" && exit 1)
	@$(CXX) --version | head -n1
	@echo "$(COLOR_GREEN)✓ MinGW-w64 found$(COLOR_RESET)"
else
	@echo This target is only available on Linux
endif

# Rebuild
.PHONY: rebuild
rebuild: clean all

# Help
.PHONY: help
help:
ifeq ($(PLATFORM),linux)
	@echo "$(COLOR_CYAN)USB Credential Provider - Makefile$(COLOR_RESET)"
	@echo ""
	@echo "$(COLOR_YELLOW)Available targets:$(COLOR_RESET)"
	@echo "  $(COLOR_GREEN)all$(COLOR_RESET)         - Build DLL and tools (default)"
	@echo "  $(COLOR_GREEN)clean$(COLOR_RESET)       - Remove build artifacts"
	@echo "  $(COLOR_GREEN)rebuild$(COLOR_RESET)     - Clean and rebuild everything"
	@echo "  $(COLOR_GREEN)check-mingw$(COLOR_RESET) - Verify MinGW-w64 installation"
	@echo "  $(COLOR_GREEN)help$(COLOR_RESET)        - Show this help message"
	@echo ""
else
	@echo USB Credential Provider - Makefile
	@echo.
	@echo Available targets:
	@echo   all         - Build DLL and tools (default)
	@echo   clean       - Remove build artifacts
	@echo   rebuild     - Clean and rebuild everything
	@echo   help        - Show this help message
	@echo.
endif

# Phony targets
.PHONY: all clean rebuild help banner success dirs check-mingw
