# Project Name
TARGET = OverdriveUnit

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

#Include Paths
C_INCLUDES += -Isrc
