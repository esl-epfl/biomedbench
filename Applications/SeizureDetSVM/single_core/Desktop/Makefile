APP = SeizDetSVM

BUILD_DIR   ?= build
SRC_DIR	    ?= Src
INC_DIR		?= Inc
INC_LIB_DIR ?= Inc/lib

GCC_FOLDER 	?= /usr/bin

# C compiler and flags
CC			:= $(GCC_FOLDER)/gcc-9 				# ATTENTION: change that to your gcc version
C_FLAGS = -O3 -Wall -I$(INC_DIR) -I$(INC_LIB_DIR) -std=c99

# C++ compiler and flags
CC_CPP		:= $(GCC_FOLDER)/g++-9 				# ATTENTION: change that to your g++ version
CPP_FLAGS = -O3 -Wall -I$(INC_DIR) -I$(INC_LIB_DIR) -std=c++14

# Linker flags
LD_FLAGS = -lm

# Find recursively all .cpp files in SRC_DIR
CPP_SRCS := $(shell find $(SRC_DIR) -type f -name '*.cpp')
OBJS_CPP := $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(CPP_SRCS))

# Find recursively all .c files in SRC_DIR
C_SRCS := $(shell find $(SRC_DIR) -type f -name '*.c')
OBJS_C := $(patsubst %.c, $(BUILD_DIR)/%.o, $(C_SRCS))


$(BUILD_DIR)/$(APP): $(OBJS_C) $(OBJS_CPP)
	@mkdir -p $$(dirname $@)
	$(CC) $(OBJS_C) $(OBJS_CPP) $(LD_FLAGS) -o $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $$(dirname $@)
	$(CC) $(C_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $$(dirname $@)
	$(CC_CPP) $(CPP_FLAGS) -c $< -o $@

all:
	$(MAKE)

run:
	./$(BUILD_DIR)/$(APP)

.PHONY: clean

info:
	@echo " make all: compiles all into build folder  -  make clean: cleans the build folder "
	@echo " Srcs: $(C_SRCS) $(CPP_SRCS) \n Outs: $(OBJS_C) $(OBJS_CPP)"

clean:
	rm -rf $(BUILD_DIR)
