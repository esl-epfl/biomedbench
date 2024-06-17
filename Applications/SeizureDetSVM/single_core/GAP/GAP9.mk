CORES = 1

APP = seizdetsvm

APP_SRCS = $(shell find Src -type f -regex ".*\.c" -not -path "./build/*")
APP_SRCS_CXX = $(shell find Src -type f -regex ".*\.cpp" -not -path "./build/*")

APP_CFLAGS = -O3 -std=c++14 -w -flto -fpermissive -I Inc -I Inc/lib -DNUM_CORES=$(CORES) -DSERIAL_AVAILABLE -DGAP9
APP_LDFLAGS = -lm 

platform = board	#or gvsoc

PMSIS_OS = freertos

include $(RULES_DIR)/pmsis_rules.mk
