LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE     := libriru_fgounlockfps
LOCAL_LDLIBS += -ldl -llog
LOCAL_LDFLAGS := -Wl
LOCAL_SRC_FILES:= main.cpp riru.c other.cpp
include $(BUILD_SHARED_LIBRARY)