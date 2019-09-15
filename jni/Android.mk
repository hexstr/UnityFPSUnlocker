LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_CPPFLAGS += $(CPPFLAGS)
LOCAL_CPPFLAGS += -fvisibility=hidden
LOCAL_MODULE     := libriru_fgounlockfps
LOCAL_LDLIBS += -ldl -llog
LOCAL_LDFLAGS := -Wl
LOCAL_SRC_FILES:= main.cpp riru.c fake_dlfcn.c
#LOCAL_CPPFLAGS += -mllvm -fla -mllvm -sub -mllvm -sobf -mllvm -bcf
include $(BUILD_SHARED_LIBRARY)