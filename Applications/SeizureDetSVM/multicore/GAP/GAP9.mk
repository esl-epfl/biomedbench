APP = seizdetsvm

APP_SRCS = $(shell find Src -type f -regex ".*\.c" -not -path "./build/*")
APP_SRCS_CXX = $(shell find Src -type f -regex ".*\.cpp" -not -path "./build/*")

APP_CFLAGS += -O3 -std=c++14 -fpermissive -I Src -I Inc/lib -I Inc -g3 -w \
               -DSERIAL_AVAILABLE -DGAP9
APP_LDFLAGS += -lm

platform = gvsoc	#or board

include $(RULES_DIR)/pmsis_rules.mk

info:
	@echo SRC: $(APP_SRCS) $(APP_SRCS_CXX)
