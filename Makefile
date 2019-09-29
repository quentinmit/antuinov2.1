# Arduino Make file. Refer to https://github.com/sudar/Arduino-Makefile

BOARD_TAG = nano
BOARD_SUB = atmega328old
ISP_PROG = stk500v1
USER_LIB_PATH := $(realpath libraries)
include $(ARDMK_DIR)/Arduino.mk
