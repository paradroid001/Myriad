#
# Outer makefile.
# Mainly for host OS detection
# If this isn't used, you need to pass HOST_PLATFORM (WINDOWS|LINUX|MACOSX) to the child makefiles.
#

# Detect Host
# Also defines "PLATFORM_SHELL"
ifeq ($(OS),Windows_NT)
	BUILD_OS = windows
	ifndef PLATFORM_SHELL
		PLATFORM_SHELL = cmd
	endif
else
	UNAMEOS = $(shell uname)
	ifeq ($(UNAMEOS),Linux)
		BUILD_OS = linux
	endif
	ifeq ($(UNAMEOS),FreeBSD)
		BUILD_OS = BSD
	endif
	ifeq ($(UNAMEOS),OpenBSD)
		BUILD_OS = BSD
	endif
	ifeq ($(UNAMEOS),NetBSD)
		BUILD_OS = BSD
	endif
	ifeq ($(UNAMEOS),DragonFly)
		BUILD_OS = BSD
	endif
	ifeq ($(UNAMEOS),Darwin)
		BUILD_OS = macosx
	endif
	ifndef PLATFORM_SHELL
		PLATFORM_SHELL = sh
	endif
endif

# Other options: Desktop, Web. Android
RELEASE_PLATFORM ?= Desktop
TARGET_ARCH		 ?= x86_64
TARGET_OS		 ?= linux
PROJECT ?= Engine
MODE    ?= Release
CONFIG = TARGET_ARCH=$(TARGET_ARCH) \
		 TARGET_OS=$(TARGET_OS) \
		 BUILD_OS=$(BUILD_OS) \
		 PLATFORM_SHELL=$(PLATFORM_SHELL) \
		 RELEASE_PLATFORM=$(RELEASE_PLATFORM) \
		 BUILD_MODE=$(MODE)


.PHONY: all clean

all:
	cd $(PROJECT) && make $(CONFIG) all

clean:
	cd $(PROJECT) && make $(CONFIG) clean
