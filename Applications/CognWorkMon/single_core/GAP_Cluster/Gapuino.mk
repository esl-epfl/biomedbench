APP = CognWorkMon
	
APP_SRCS  =  $(shell find Src -type f -regex ".*\.c" -not -path "./build/*")
	
APP_CFLAGS += -O3 -g3 -w -IInc/fixmath -IInc/Data -IInc -DGAPUINO
APP_LDFLAGS += -lm
platform = board	#or gvsoc
#PMSIS_OS = freertos

include $(RULES_DIR)/pmsis_rules.mk
