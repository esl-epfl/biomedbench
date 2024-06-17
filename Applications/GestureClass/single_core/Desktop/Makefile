APP = sEMG_BSS

BUILD_DIR     ?=build
SRC_DIR	      ?=./Src

GCC_FOLDER 	?=/usr/bin
CC			:=$(GCC_FOLDER)/gcc-9

C_FLAGS = -Wall -g -O3 -IInc -IInc/data


C_SRCS := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c' -or -name '*.s')
OBJS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(C_SRCS))


$(BUILD_DIR)/$(APP): $(OBJS)
	@mkdir -p $$(dirname $@)
	$(CC) $(OBJS) -o $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $$(dirname $@)
	$(CC) $(C_FLAGS) -c $< -o $@

all:
	$(MAKE)

run:
	./$(BUILD_DIR)/$(APP)

.PHONY: clean

info:
	@echo "make all: compiles all into build folder  -  make clean: cleans the build folder"
	@echo " Srcs: $(C_SRCS) \n Outs: $(OBJS) "
clean:
	rm -rf $(BUILD_DIR)
