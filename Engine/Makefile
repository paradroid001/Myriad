#
# Myriad base makefile
#

TARGET_PLATFORM			?= UNKNOWN
HOST_PLATFORM			?= UNKNOWN
LIBRARY_NAME			?= myriad
#DEBUG or RELEASE
BUILD_MODE				?= RELEASE
SUB_MAKEFILE			?= UNKNOWN
BIN_DIR					?= bin
OBJ_DIR					?= obj
INC_DIR					?= include
SRC_DIR					?= src
EXT_DIR					?= ext

OUTPUT_DIR				?= $(BIN_DIR)/$(TARGET_PLATFORM)/$(BUILD_MODE)/$(HOST_PLATFORM)

# Detect Host
# Also defines "PLATFORM_SHELL"
ifeq ($(OS),Windows_NT)
	HOST_PLATFORM = WINDOWS
	ifndef PLATFORM_SHELL
		PLATFORM_SHELL = cmd
	endif
else
	UNAMEOS = $(shell uname)
	ifeq ($(UNAMEOS),Linux)
		HOST_PLATFORM = LINUX
	endif
	ifeq ($(UNAMEOS),FreeBSD)
		HOST_PLATFORM = BSD
	endif
	ifeq ($(UNAMEOS),OpenBSD)
		HOST_PLATFORM = BSD
	endif
	ifeq ($(UNAMEOS),NetBSD)
		HOST_PLATFORM = BSD
	endif
	ifeq ($(UNAMEOS),DragonFly)
		HOST_PLATFORM = BSD
	endif
	ifeq ($(UNAMEOS),Darwin)
		HOST_PLATFORM = OSX
	endif
	ifndef PLATFORM_SHELL
		PLATFORM_SHELL = sh
	endif
endif

ifeq ($(TARGET_PLATFORM), PLATFORM_DESKTOP)
	SUB_MAKEFILE := Makefile.DESKTOP
endif
ifeq ($(TARGET_PLATFORM), PLATFORM_WEB)
	SUB_MAKEFILE := Makefile.WEB
endif
ifeq ($(TARGET_PLATFORM), PLATFORM_ANDROID)
	SUB_MAKEFILE := Makefile.ANDROID
endif

all:
ifeq ($(SUB_MAKEFILE),UNKNOWN)	
	@echo "No makefile to build for $(TARGET_PLATFORM)"
else
	@echo Building $(LIBRARY_NAME) [$(BUILD_MODE)] for $(TARGET_PLATFORM) on $(HOST_PLATFORM)
	@echo Includes=$(INC_DIR)
	@echo Output=$(OUTPUT_DIR)
	@make -f $(SUB_MAKEFILE) $@ \
		HOST_PLATFORM=$(HOST_PLATFORM) \
		LIBRARY_NAME=$(LIBRARY_NAME) \
		BUILD_MODE=$(BUILD_MODE) \
		INC_DIR=$(INC_DIR) \
		OUTPUT_DIR=$(OUTPUT_DIR)
endif