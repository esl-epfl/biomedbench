APP = SeizDetCNN

BUILD_DIR   ?= build
SRC_DIR	    ?= Src
INC_DIR		?= Inc

GCC_FOLDER 	?= /usr/bin
CC			:= $(GCC_FOLDER)/gcc-11 				# ATTENTION: change that to your g++ version

CPP_FLAGS = -O3 -Wall -I$(INC_DIR) -std=c99
LD_FLAGS = -lm

# Find recursively all .c files in SRC_DIR
C_SRCS := $(shell find $(SRC_DIR) -type f -name '*.c')
OBJS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(C_SRCS))



$(BUILD_DIR)/$(APP): $(OBJS)
	@mkdir -p $$(dirname $@)
	$(CC) $(OBJS) $(LD_FLAGS) -o $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $$(dirname $@)
	$(CC) $(CPP_FLAGS) -c $< -o $@

all:
	$(MAKE)

run:
	./$(BUILD_DIR)/$(APP)

.PHONY: clean

info:
	@echo " make all: compiles all into build folder  -  make clean: cleans the build folder "
	@echo " Srcs: $(C_SRCS) \n Outs: $(OBJS) "

clean:
	rm -rf $(BUILD_DIR)
