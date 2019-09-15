#include <jni.h>
#include <pthread.h>
#include <unistd.h>
#include "riru.h"
#include "logger.h"
/*import fake_dlfcn*/
#include <string.h>
#include <stdlib.h>
extern "C" {
extern void *fake_dlopen(const char *filename, int flags);
extern void *fake_dlsym(void *handle, const char *symbol);
extern void fake_dlclose(void *handle);
}

int is_app_need_hook(JNIEnv *, jstring);
int getPackageName();
static char package_name[256];
const char* package = "com.aniplex.fategrandorder";
static int enable_hook;

typedef void *il2cpp_resolve_icall(const char*);
typedef void set_targetFrameRate(int);

void* thereisnothing(void *args) {
    LOGD("*****new thread: [%d]*****", gettid());
    sleep(10);
    LOGD("***** begin *****");
    if (getPackageName ()) {
        LOGD("packageName: %s", package_name);
        void* handle = fake_dlopen(package_name, 0);
        if (handle) {
            void* f_set_targetFrameRate = 0;
            void *f_il2cpp_resolve_icall = fake_dlsym(handle, "il2cpp_resolve_icall");
            f_set_targetFrameRate = ((il2cpp_resolve_icall*)f_il2cpp_resolve_icall)("UnityEngine.Application::set_targetFrameRate(System.Int32)");
            ((set_targetFrameRate*)f_set_targetFrameRate)(60);
            LOGD("called set_targetFrameRate");
        } else {
            LOGE("fake dlopen failed");
        }
    }
    else {
        LOGE("il2cpp.so not found");
    }
    LOGD ("***** finish *****");
    return ((void *)0);
}

extern "C" {
    __attribute__((visibility("default")))
        void nativeForkAndSpecializePre(
            JNIEnv *env, jclass clazz, jint *_uid, jint *gid, jintArray *gids, jint *runtime_flags,
            jobjectArray *rlimits, jint *_mount_external, jstring *se_info, jstring *se_name,
            jintArray *fdsToClose, jintArray *fdsToIgnore, jboolean *is_child_zygote,
            jstring *instructionSet, jstring *appDataDir, jstring *packageName,
            jobjectArray *packagesForUID, jstring *sandboxId) {
        // packageName, packagesForUID, sandboxId exists from Android Q
        enable_hook = is_app_need_hook(env, *appDataDir);
    }
    __attribute__((visibility("default")))
        int nativeForkAndSpecializePost(JNIEnv *env, jclass clazz, jint res) {
        if (res == 0 && enable_hook) {
            int temp;
            pthread_t ntid;
            if ((temp = pthread_create(&ntid, NULL, thereisnothing, NULL))) {
                LOGD("can't create thread");
            }
        }
        return !enable_hook;
    }
}

int is_app_need_hook(JNIEnv *env, jstring jAppDataDir) {
    if (jAppDataDir) {
        const char *appDataDir = env->GetStringUTFChars(jAppDataDir, nullptr);
        int user = 0;
        if (sscanf(appDataDir, "/data/%*[^/]/%d/%s", &user, package_name) != 2) {
            if (sscanf(appDataDir, "/data/%*[^/]/%s", package_name) != 1) {
                package_name[0] = '\0';
                return false;
            }
        }
        env->ReleaseStringUTFChars(jAppDataDir, appDataDir);
    } else {
        return false;
    }
    if (strcmp(package, package_name) == 0)
        return 1;
    return 0;
}

int getPackageName () {
    FILE *fp = fopen ("/proc/self/maps", "r");
    char line[1024];
    if (fp != NULL) {
        while (fgets (line, sizeof(line), fp)) {
            if (strstr (line, "libil2cpp.so")) {
                 strcpy(package_name, strstr(line, "/"));
                 int len = strlen(package_name);
                 package_name[strlen(package_name) - 1] = 0;
                break;
            }
        }
        fclose (fp);
        return 1;
    }
    return 0;
}