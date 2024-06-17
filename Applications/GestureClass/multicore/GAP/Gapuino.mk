APP = semg-bss-online

APP_SRCS += $(shell find src -type f -regex ".*\.c" -not -path "./build/*")
APP_CFLAGS += -I. -I./Inc

# Signal parameters 
FS_ ?= 4000		# use FS_ to avoid conflict with gap9 sdk
WIN_LEN ?= 200
N_CH ?= 16
APP_CFLAGS += -D FS_=$(FS_) -D WIN_LEN=$(WIN_LEN) -D N_CH=$(N_CH)

# Decomposition parameters
FE ?= 4
N_MU ?= 19
Q ?= 32
APP_CFLAGS += -D FE=$(FE) -D N_MU=$(N_MU) -D Q=$(Q)

# Classification parameters
N_TA ?= 4
N_CA ?= 16
N_OUT ?= 5
APP_CFLAGS += -D N_TA=$(N_TA) -D N_CA=$(N_CA) -D N_OUT=$(N_OUT)
ifdef USE_SVM
APP_CFLAGS += -D USE_SVM
endif

# Optimization flags
APP_CFLAGS += -g -O3 -DGAPUINO -flto

# Math library linking
APP_LDFLAGS += -lm 

#PMSIS_OS = freertos

include $(RULES_DIR)/pmsis_rules.mk
