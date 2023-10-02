#
# Myriad base makefile
#

TARGET_ARCH				?= x86_64
# Valid choices linux, windows, macosx
TARGET_OS				?= linux
TARGET_PLATFORM			?= $(TARGET_ARCH)-$(TARGET_OS)-$(TARGET_LIBC)
# Valid choices are Desktop, Web, Android
RELEASE_PLATFORM		?= Desktop
# The host os that we are building on (tools like rm, etc)
BUILD_OS				?= UNKNOWN
LIBRARY_NAME			?= libmyriad.so
#Debug or Release
BUILD_MODE				?= Release
SUB_MAKEFILE			?= UNKNOWN
BUILD_DIR				?= build
BIN_DIR					?= bin
OBJ_DIR					?= obj
INC_DIR					?= .
SRC_DIR					?= src
EXT_DIR					?= ext

OUTPUT_DIR				?= $(BUILD_DIR)/$(RELEASE_PLATFORM)/$(BUILD_MODE)/$(TARGET_PLATFORM)

ifeq ($(RELEASE_PLATFORM), Desktop)
	SUB_MAKEFILE := Makefile.DESKTOP
endif
ifeq ($(RELEASE_PLATFORM), Web)
	SUB_MAKEFILE := Makefile.WEB
endif
ifeq ($(RELEASE_PLATFORM), Android)
	SUB_MAKEFILE := Makefile.ANDROID
endif

clean:
	@make -f $(SUB_MAKEFILE) $@ \
		TARGET_PLATFORM=$(TARGET_PLATFORM) \
		RELEASE_PLATFORM=$(RELEASE_PLATFORM) \
		BUILD_OS=$(BUILD_OS) \
		BUILD_MODE=$(BUILD_MODE) \
		INC_DIR=$(INC_DIR) \
		OUTPUT_DIR=$(OUTPUT_DIR)
all:

ifeq ($(SUB_MAKEFILE),UNKNOWN)	
	@echo "No makefile to build for $(RELEASE_PLATFORM)"
else
	@echo Building $(LIBRARY_NAME) [$(BUILD_MODE)] for $(RELEASE_PLATFORM) [$(TARGET_PLATFORM)] on $(BUILD_OS)
	@echo Includes=$(INC_DIR)
	@echo Src=$(SRC_DIR)
	@echo Output=$(OUTPUT_DIR)
endif
ifeq ($(BEAR),1)
	bear -- make -f $(SUB_MAKEFILE) $@ \
		TARGET_ARCH=$(TARGET_ARCH) \
		TARGET_OS=$(TARGET_OS) \
		TARGET_PLATFORM=$(TARGET_PLATFORM) \
		RELEASE_PLATFORM=$(RELEASE_PLATFORM) \
		BUILD_OS=$(BUILD_OS) \
		BUILD_MODE=$(BUILD_MODE) \
		SRC_DIR=$(SRC_DIR) \
		OUTPUT_DIR=$(OUTPUT_DIR) \
		BUILD_DIR=$(BUILD_DIR) \
		CXX=g++
else
	make -f $(SUB_MAKEFILE) $@ \
		TARGET_ARCH=$(TARGET_ARCH) \
		TARGET_OS=$(TARGET_OS) \
		TARGET_PLATFORM=$(TARGET_PLATFORM) \
		RELEASE_PLATFORM=$(RELEASE_PLATFORM) \
		BUILD_OS=$(BUILD_OS) \
		BUILD_MODE=$(BUILD_MODE) \
		SRC_DIR=$(SRC_DIR) \
		OUTPUT_DIR=$(OUTPUT_DIR) \
		BUILD_DIR=$(BUILD_DIR)
endif