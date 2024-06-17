
APP = coughDetect
APP_SRCS = $(shell find Src -type f -regex ".*\.c" -not -path "./BUILD/*")

APP_CFLAGS = -O3 -std=gnu11 -w -fpermissive -fno-math-errno -ffast-math -I Inc -DGAP9
APP_LDFLAGS = -lm

platform = gvsoc	#or board
PMSIS_OS = freertos

include $(RULES_DIR)/pmsis_rules.mk

info:
	@echo SRC: $(APP_SRCS)
