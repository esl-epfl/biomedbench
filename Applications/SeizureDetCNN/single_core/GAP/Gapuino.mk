CORES = 1

APP = SeizDetCNN
APP_SRCS = $(shell find Src -type f -regex ".*\.c" -not -path "./build/*")

APP_CFLAGS = -O3 -g3 -w -flto -fpermissive -I Inc -DNUM_CORES=$(CORES) -DGAPUINO
APP_LDFLAGS = -lm 

platform = board	#or gvsoc
PMSIS_OS = freertos

include $(RULES_DIR)/pmsis_rules.mk
