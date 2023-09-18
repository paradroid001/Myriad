SRC_DIR			?= src
BUILD_DIR		?= build
OBJ_DIR			:= build/obj
MYRIAD_DIR		?= ../Engine
MYRIAD_LIB		?= $(MYRIAD_DIR)/build/PLATFORM_DESKTOP/RELEASE/WINDOWS/bin
TARGET			?= sample
CXX				?= g++
CFLAGS			?= -std=c++17
LFLAGS			?= -L $(MYRIAD_LIB)

OBJECTS			:= $(OBJ_DIR)/main.o

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

.PHONY: setup

setup:
	@echo $(CFLAGS)
CFLAGS += -I $(MYRIAD_DIR)/src/include
LFLAGS += -l myriad

all: setup $(BUILD_DIR)/$(TARGET)

# LFLAGS Must come last
$(BUILD_DIR)/$(TARGET): $(OBJECTS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) -o $(BUILD_DIR)/$(TARGET) $(OBJECTS) $(LFLAGS)
	@cp $(MYRIAD_LIB)/* $(BUILD_DIR)
	

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo Building $@
	@mkdir -p $(OBJ_DIR)
	$(CXX) -MMD -MP -c $(CFLAGS) $< -o $@


  
