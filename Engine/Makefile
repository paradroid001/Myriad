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
SRC_DIR					?= src
OUTPUT_DIR				?= $(BUILD_DIR)/$(RELEASE_PLATFORM)/$(BUILD_MODE)/$(TARGET_PLATFORM)
BIN_DIR					:= $(OUTPUT_DIR)/bin
OBJ_DIR					:= $(OUTPUT_DIR)/obj
LIB_DIR					:= $(OUTPUT_DIR)/lib

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))
SOURCES		:=$(call rwildcard,$(SRC_DIR),*.cpp)
OBJECTS		:=$(patsubst $(SRC_DIR)/%, $(OBJ_DIR)/%, $(patsubst %.cpp, %.o, $(SOURCES)))


ifeq ($(BUILD_OS),windows)
    COPY = copy
    MKDIR = mkdir -p
    RM = rm -rf
endif
ifeq ($(BUILD_OS),linux)
    COPY = cp
    MKDIR = mkdir -p
    RM = rm -rf
endif

#valid values true, false
USE_REMOTERY=false


ifeq ($(RELEASE_PLATFORM), Desktop)
	SUB_MAKEFILE := Makefile.DESKTOP
endif
ifeq ($(RELEASE_PLATFORM), Web)
	SUB_MAKEFILE := Makefile.WEB
endif
ifeq ($(RELEASE_PLATFORM), Android)
	SUB_MAKEFILE := Makefile.ANDROID
endif

.PHONY: clean cleanlibs all test

#this line means all env variables / make flags are exported to subshells.
export

clean:
	@make -f $(SUB_MAKEFILE) $@

cleanlibs:
	@make -f $(SUB_MAKEFILE) $@

test:
	@make -C test -f Makefile.test all

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
	bear -- make -f $(SUB_MAKEFILE) $@
#	bear -- make -f $(SUB_MAKEFILE) $@ \
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
	make -f $(SUB_MAKEFILE) $@
#	make -f $(SUB_MAKEFILE) $@ \
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