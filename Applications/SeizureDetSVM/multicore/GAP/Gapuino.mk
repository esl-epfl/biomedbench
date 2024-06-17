APP = seizdetsvm

APP_SRCS = $(shell find src -type f -regex ".*\.c" -not -path "./build/*")
APP_SRCS_CXX = $(shell find src -type f -regex ".*\.cpp" -not -path "./build/*")

APP_CFLAGS += -O3 -std=c++14 -fpermissive -I src -I src/lib -g3 -w \
               -DSERIAL_AVAILABLE -DGAPUINO
APP_LDFLAGS += -lm

platform = gvsoc	#or board
#PMSIS_OS=freertos

include $(RULES_DIR)/pmsis_rules.mk

info:
	@echo SRC: $(APP_SRCS) $(APP_SRCS_CXX)
