LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE     := libriru_fgofpsunlocker
LOCAL_LDLIBS += -ldl -llog
LOCAL_LDFLAGS := -Wl
LOCAL_SRC_FILES:= main.c riru.c fake_dlfcn.c
include $(BUILD_SHARED_LIBRARY)