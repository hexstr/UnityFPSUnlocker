#ifndef LOGGER_HEADER
#define LOGGER_HEADER

#define LOG_TAG "UnityFPSUnlocker"
#include <android/log.h>

#define logger(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

#endif // LOGGER_HEADER