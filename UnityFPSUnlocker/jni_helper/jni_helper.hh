#pragma once

#include <jni.h>

#include <vector>

#include "utility/logger.hh"

#define JNIEX(env, name)          \
    if (name == nullptr) {        \
        ERROR("nullptr " #name);  \
        JNIExceptionHandler(env); \
        return nullptr;           \
    }

namespace JNIHelper {
    struct Mode {
        int id;
        int width;
        int height;
        float refresh_rate;
    };

    jobject GetDisplay(JNIEnv* env, jobject activity);
    jobject GetActivity(JNIEnv* env);
    jint GetPreferredModeId(JNIEnv* env, jobject activity);
    std::vector<Mode> GetSupportedModes(JNIEnv* env, jobject activity);
    void SetPreferredMode(JNIEnv* env, jobject activity, jint mode);

    void JNIExceptionHandler(JNIEnv* env);
} // namespace JNIHelper