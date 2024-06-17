APP = hcl

APP_SRCS  =  $(shell find Src -type f -regex ".*\.c" -not -path "./build/*")

CORES ?= 8
CTARGET ?= 1 

APP_CFLAGS += -DTARGET=$(CTARGET) -DNUM_CORES=$(CORES) -O3 -g3 -IInc -w -fpermissive -DGAP9
platform = gvsoc	#or board
PMSIS_OS=freertos

include $(RULES_DIR)/pmsis_rules.mk

