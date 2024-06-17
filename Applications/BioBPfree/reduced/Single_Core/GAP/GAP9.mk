APP = BioBPfree_conv_block

APP_SRCS	= $(shell find Src -type f -regex ".*\.c" -not -path "./build/*")

CORES ?= 1
CTARGET ?= 0 

APP_CFLAGS += -DTARGET=$(CTARGET) -DNUM_CORES=$(CORES) -DPULP_V2 -ffast-math -O3 -g3 -IInc -IInc/training_lib -w -std=gnu11 -DGAP9
APP_LDFLAGS += -lm

PMSIS_OS = freertos

platform = board	#or gvsoc

include $(RULES_DIR)/pmsis_rules.mk
