LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE     := libriru_bgofpsunlocker
LOCAL_LDLIBS += -ldl -llog
LOCAL_LDFLAGS := -Wl
LOCAL_SRC_FILES:= main.cpp riru.c
LOCAL_CPPFLAGS += -fexceptions -fvisibility=hidden
include $(BUILD_SHARED_LIBRARY)