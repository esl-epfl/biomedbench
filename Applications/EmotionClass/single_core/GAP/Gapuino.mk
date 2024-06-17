APP = HeartBeatClass

APP_SRCS  =  $(shell find Src -type f -regex ".*\.c" -not -path "./build/*")

CORES ?= 1
CTARGET ?= 0 

APP_CFLAGS += -DTARGET=$(CTARGET) -DNUM_CORES=$(CORES) -DPULP_V2 -DGAPUINO -O3 -g3 -IInc -w
APP_LDFLAGS += -lm

platform = board	#or gvsoc

PMSIS_OS = freertos

include $(RULES_DIR)/pmsis_rules.mk
