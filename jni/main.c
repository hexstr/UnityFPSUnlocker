#include <jni.h>
#include <pthread.h>
#include <unistd.h>

// import logcat
#include "android/log.h"
#define LOG_TAG "ethereal"
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
// import fake_dlfcn
#include <string.h>
#include <stdlib.h>
extern void *fake_dlopen(const char *filename, int flags);
extern void *fake_dlsym(void *handle, const char *symbol);
extern void fake_dlclose(void *handle);


int IsAppNeedHook(JNIEnv *, jstring);
const char* PackageName = "/data/user/0/com.aniplex.fategrandorder.en";
static int NeedHook;

// il2cpp api
typedef void* (*il2cpp_resolve_icall_f)(const char*);
typedef void (*set_targetFrameRate_f)(int);

static il2cpp_resolve_icall_f il2cpp_resolve_icall;
static set_targetFrameRate_f set_targetFrameRate;

void* thereisnothing(void *args) {
    LOGD("***** new thread *****");
    sleep(10);
    LOGD("***** begin *****");
    void* handle = fake_dlopen("libil2cpp.so", 0);
    if (handle) {
        // Read configuration
        int framerate = 60;
        FILE* fp = fopen("/data/local/tmp/unlocker","r");
        if (fp != NULL) {
            char chr [4];
            fgets(chr, 4, fp);
            framerate = atoi(chr);
            fclose(fp);
        }
        LOGD("set framerate: %d", framerate);
        il2cpp_resolve_icall = (il2cpp_resolve_icall_f)fake_dlsym(handle, "il2cpp_resolve_icall");
        set_targetFrameRate = (set_targetFrameRate_f)il2cpp_resolve_icall("UnityEngine.Application::set_targetFrameRate(System.Int32)");
        set_targetFrameRate(framerate);
        LOGD("set_targetFrameRate called");
    } else {
        LOGD("fake dlopen failed");
    }
    LOGD ("***** finish *****");
    return ((void *)0);
}

__attribute__((visibility("default")))
    void nativeForkAndSpecializePre(
        JNIEnv *env, jclass clazz, jint *_uid, jint *gid, jintArray *gids, jint *runtime_flags,
        jobjectArray *rlimits, jint *_mount_external, jstring *se_info, jstring *se_name,
        jintArray *fdsToClose, jintArray *fdsToIgnore, jboolean *is_child_zygote,
        jstring *instructionSet, jstring *appDataDir, jstring *packageName,
        jobjectArray *packagesForUID, jstring *sandboxId) {
    // packageName, packagesForUID, sandboxId exists from Android Q
    if (*appDataDir) {
        const char* jAppDataDir = (*env)->GetStringUTFChars(env, *appDataDir, NULL);
        if (strcmp(PackageName, jAppDataDir) == 0) {
            NeedHook = 1;
        }
        else {
            NeedHook = 0;
        }
        (*env)->ReleaseStringUTFChars(env, *appDataDir, jAppDataDir);
    }
    else {
        NeedHook = 0;
    }
}
__attribute__((visibility("default")))
    int nativeForkAndSpecializePost(JNIEnv *env, jclass clazz, jint res) {
    if (res == 0 && NeedHook) {
        int temp;
        pthread_t ntid;
        if ((temp = pthread_create(&ntid, NULL, thereisnothing, NULL))) {
            LOGD("can't create thread");
        }
    }
    return !NeedHook;
}