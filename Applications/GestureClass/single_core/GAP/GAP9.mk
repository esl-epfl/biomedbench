APP = HeartBeatClass

APP_SRCS	= $(shell find Src -type f -regex ".*\.c" -not -path "./build/*")

CORES ?= 1
CTARGET ?= 0 

APP_CFLAGS += -DTARGET=$(CTARGET) -DNUM_CORES=$(CORES) -DPULP_V2 -O3 -g3 -IInc -IInc/data -w -std=gnu11 -DGAP9

PMSIS_OS = freertos

platform = board	#or gvsoc

include $(RULES_DIR)/pmsis_rules.mk
