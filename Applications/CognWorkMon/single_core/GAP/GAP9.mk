# TORUN: 	make --file=Gapuino.mk clean all run 

APP = CognWorkMon
	
APP_SRCS  =  $(shell find Src -type f -regex ".*\.c" -not -path "./build/*")

INC=-IInc/fixmath -IInc
	
APP_CFLAGS += -O3 -g3 -w $(INC)  -DGAP9
APP_LDFLAGS += -lm
platform = board	#or gvsoc
PMSIS_OS=freertos

include $(RULES_DIR)/pmsis_rules.mk
