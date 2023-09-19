#
# Outer makefile.
# Mainly for host OS detection
# If this isn't used, you need to pass HOST_PLATFORM (WINDOWS|LINUX|MACOSX) to the child makefiles.
#

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

# Other options: PLATFORM_WEB, PLATFORM_ANDROID
TARGET_PLATFORM ?= PLATFORM_DESKTOP
PROJECT ?= Engine
MODE    ?= RELEASE
CONFIG = HOST_PLATFORM=$(HOST_PLATFORM) \
		 PLATFORM_SHELL=$(PLATFORM_SHELL) \
		 TARGET_PLATFORM=$(TARGET_PLATFORM) \
		 BUILD_MODE=$(MODE)


.PHONY: all clean

all:
	cd $(PROJECT) && make $(CONFIG) all

clean:
	cd $(PROJECT) && make $(CONFIG) clean
