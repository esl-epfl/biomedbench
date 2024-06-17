APP = CNN_Training_Adam

BUILD_DIR     ?= build
SRC_DIR	      ?= Src

GCC_FOLDER 	?= /usr/bin
CC			:= $(GCC_FOLDER)/gcc-11

C_FLAGS = -O3 -Wall -IInc -IInc/C_training_lib
LD_FLAGS = -lm
DEBUG_FLAGS =

C_SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(C_SRCS))


$(BUILD_DIR)/$(APP): $(OBJS)
	@mkdir -p $$(dirname $@)
	$(CC) $(OBJS) $(LD_FLAGS) -o $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $$(dirname $@)
	$(CC) $(DEBUG_FLAGS) $(C_FLAGS) -c $< -o $@

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
