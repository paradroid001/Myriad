TARGET_ARCH			?= x86_64
TARGET_OS			?= linux
TARGET_PLATFORM		?= $(TARGET_ARCH)-$(TARGET_OS)-$(TARGET_LIBC)
SRC_DIR				?= src
BUILD_MODE			?= Release
BUILD_OS			?= linux
RELEASE_PLATFORM 	?= Desktop
BUILD_DIR			?= build
VARIANT_PATH		?= $(RELEASE_PLATFORM)/$(BUILD_MODE)/$(TARGET_PLATFORM)
OBJ_DIR				:= $(BUILD_DIR)/obj/$(VARIANT_PATH)
MYRIAD_DIR			?= ../Engine
MYRIAD_LIB			:= $(MYRIAD_DIR)/build/$(VARIANT_PATH)/bin
TARGET				?= sample
CXX					= zig c++ -target $(TARGET_PLATFORM)
CFLAGS				?= -std=c++17 -I ../Engine/vendor/spdlog/include -I ../Engine/vendor/raylib/src
#LFLAGS				+= -L.
#LFLAGS				+= -L $(MYRIAD_LIB)
OBJECTS				:= $(OBJ_DIR)/main.o $(OBJ_DIR)/Dot.o

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

ifeq ($(TARGET_OS),windows)
	CFLAGS += -DMYR_PLATFORM_WINDOWS
	LFLAGS += -l myriad
endif
ifeq ($(TARGET_OS),linux)
	CFLAGS += -DMYR_PLATFORM_LINUX
	LFLAGS += -l myriad
endif

CFLAGS += -I $(MYRIAD_DIR)/src
LFLAGS += -L $(BUILD_DIR)/$(VARIANT_PATH)


.PHONY: setup

setup:
	@echo $(CXX)
	@echo $(CFLAGS)
    

all: setup $(BUILD_DIR)/$(TARGET)

# LFLAGS Must come last
$(BUILD_DIR)/$(TARGET): $(OBJECTS)
	@mkdir -p $(BUILD_DIR)/$(VARIANT_PATH)
	@cp $(MYRIAD_LIB)/* $(BUILD_DIR)/$(VARIANT_PATH)
	$(CXX) -o $(BUILD_DIR)/$(VARIANT_PATH)/$(TARGET) $(OBJECTS) $(LFLAGS)
	

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) -MMD -MP -c $(CFLAGS) $< -o $@

clean:
	@rm -rf build
