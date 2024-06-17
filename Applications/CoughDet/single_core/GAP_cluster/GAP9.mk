
APP = coughDetect
APP_SRCS = $(shell find Src -type f -regex ".*\.c" -not -path "./BUILD/*")

APP_CFLAGS = -O3 -std=gnu11 -w -fpermissive -I Inc -DPULP -ffast-math -DGAP9 #-fno-math-errno
APP_LDFLAGS = -lm

platform = gvsoc	#or board
PMSIS_OS = freertos

include $(RULES_DIR)/pmsis_rules.mk

info:
	@echo SRC: $(APP_SRCS)
