# Project Name
TARGET = GyroBass

#C Include Paths
C_INCLUDES += -Isrc \
                -Isrc/FreeRTOS/include \
                -Isrc/FreeRTOS/portable

C_SOURCES += src/FreeRTOS/tasks.c \
             src/FreeRTOS/queue.c \
             src/FreeRTOS/list.c \
             src/FreeRTOS/timers.c \
             src/FreeRTOS/event_groups.c \
             src/FreeRTOS/stream_buffer.c \
             src/FreeRTOS/portable/port.c \
             src/FreeRTOS/portable/heap_4.c

#Sources
CPP_SOURCES = src/main.cpp \
              src/sensors/CustomGyro.cpp \
              src/dsp/DynamicStateVariableFilter.cpp

# Library Location - Poining to gitsubmodules
LIBDAISY_DIR = libs/libDaisy/
DAISYSP_DIR = libs/DaisySP/

#Core location
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/Core
include $(SYSTEM_FILES_DIR)/Makefile