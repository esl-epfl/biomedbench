APP = seizdetcnn
APP_SRCS = $(shell find Src -type f -regex ".*\.c" -not -path "./build/*")

APP_CFLAGS = -O3 -std=c++14 -w -flto -fpermissive -I Inc -DGAPUINO -DPULP
APP_LDFLAGS = -lm 

platform = gvsoc	#or board
PMSIS_OS = freertos

include $(RULES_DIR)/pmsis_rules.mk

info:
	@echo SRC: $(APP_SRCS)
