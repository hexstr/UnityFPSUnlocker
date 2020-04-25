LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE     := libriru_fgofpsunlocker4en
LOCAL_LDLIBS += -ldl -llog
LOCAL_LDFLAGS := -Wl
LOCAL_SRC_FILES:= main.cpp
LOCAL_CPPFLAGS += -fexceptions -fvisibility=hidden
include $(BUILD_SHARED_LIBRARY)