#ifndef LOGGER_HEADER
#define LOGGER_HEADER

#define LOG_TAG "riru_hide"
#include <android/log.h>

#ifdef DEBUG
#define RIRU_LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#else
#define RIRU_LOGD(...)
#endif
#define RIRU_LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define RIRU_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define PLOGE(fmt, args...) RIRU_LOGE(fmt " failed with %d: %s", ##args, errno, strerror(errno))

#endif // LOGGER_HEADER