#include <jni.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "riru.h"
#include "main.h"
#include "other.h"

static int enable_hook;
MonoImage * mono_image_open_from_data_internal (char *data, guint32 data_len, gboolean need_copy, MonoImageOpenStatus *status, gboolean refonly, gboolean metadata_only, const char *name) {
    MonoCLIImageInfo *iinfo;
    MonoImage *image;
    char *datac;

    if (!data || !data_len) {
        if (status)
            *status = MONO_IMAGE_IMAGE_INVALID;
        return NULL;
    }
    datac = data;
    if (need_copy) {
        datac = (char *)g_malloc (data_len);
        if (!datac) {
            if (status)
                *status = MONO_IMAGE_ERROR_ERRNO;
            return NULL;
        }
        memcpy (datac, data, data_len);
    }

    image = (MonoImage*)g_new0 (872);
    image->raw_data = datac;
    image->raw_data_len = data_len;
    image->raw_data_allocated = need_copy;
    image->name = (name == NULL) ? g_strdup_printf ("data-%p", datac) : g_strdup(name);
    iinfo = (MonoCLIImageInfo*)g_new0 (396);
    image->image_info = iinfo;
    image->ref_only = refonly;
    image->metadata_only = metadata_only;
    image->ref_count = 1;

    image = do_mono_image_load (image, status, 1, 1);
    if (image == NULL)
        return NULL;

    return register_image (image);
}

int inject (long mono_addr) {
	/*define mono func*/
    //mono_get_root_domain
    long mono_get_root_domain_addr = mono_addr + 0x00;
    //mono_thread_attach
    long mono_thread_attach_addr = mono_addr + 0x00;
    //mono_assembly_load_from_full
    long mono_assembly_load_from_full_addr = mono_addr + 0x00;
    //mono_class_from_name
    long mono_class_from_name_addr = mono_addr + 0x00;
    //mono_object_new
    long mono_object_new_addr = mono_addr + 0x00;
    //mono_runtime_object_init
    long mono_runtime_object_init_addr = mono_addr + 0x00;
    //g_malloc
    g_malloc = (void*(*)(unsigned long))(mono_addr + 0x00);
    //g_new0
    g_new0 = (void*(*)(unsigned long))(mono_addr + 0x00);
    //g_strdup_printf
    g_strdup_printf = (char*(*)(const char *format, ...))(mono_addr + 0x00);
    //g_strdup
    g_strdup = (char*(*)(const char *str)) (mono_addr + 0x00);
    //register_image
    register_image = (MonoImage*(*)(MonoImage *image))(mono_addr + 0x00);
    //do_mono_image_load
    do_mono_image_load = (MonoImage*(*)(MonoImage *image, MonoImageOpenStatus *status, gboolean care_about_cli, gboolean care_about_pecoff))(mono_addr + 0x00);

    mono_get_root_domain_t mono_get_root_domain = (mono_get_root_domain_t)(mono_get_root_domain_addr);
    mono_thread_attach_t mono_thread_attach = (mono_thread_attach_t)(mono_thread_attach_addr);
    mono_assembly_load_from_full_t mono_assembly_load_from_full = (mono_assembly_load_from_full_t)(mono_assembly_load_from_full_addr);
    mono_class_from_name_t mono_class_from_name = (mono_class_from_name_t)(mono_class_from_name_addr);
    mono_object_new_t mono_object_new = (mono_object_new_t)(mono_object_new_addr);
    mono_runtime_object_init_t mono_runtime_object_init = (mono_runtime_object_init_t)(mono_runtime_object_init_addr);
	/*end*/

    mono_thread_attach (mono_get_root_domain ());
    MonoImageOpenStatus status;
    int D_size = sizeof (raw) / sizeof(raw[0]);
    void* image = mono_image_open_from_data_internal ((char*)raw, D_size, 1, &status, 0, 0, NULL);
    mono_assembly_load_from_full (image, "unlockFPS", &status, 0);
    void* pClass = mono_class_from_name (image, "unlockFPS", "_44Ot44O844OA");
    void* method = mono_object_new (mono_get_root_domain (), pClass);
    mono_runtime_object_init (method);
    LOGD ("ojbk");
    return 0;
}

void* thereisnothing(void *args) {
    LOGD("***** new thread: [%d] *****", gettid());
    sleep(10);
    LOGD("***** begin *****");
    int count = 0;
    while (true)
    {
        long baseAddr = get_module_base ();
        if (baseAddr != 0)
        {
            LOGD ("baseAddr %02lX", baseAddr);
            inject (baseAddr);
            break;
        }
        else
        {
            count++;
            sleep (1);
            if (count > 20)
            {
                LOGD ("**** find module base address failed ****");
                break;
            }
        }
    }
    LOGD ("***** finish *****");
    return 0;
}

void nativeForkAndSpecialize(int res, int enable_hook) {
    if (res == 0 && enable_hook) {
        int temp;
        pthread_t ntid;
        if ((temp = pthread_create(&ntid, NULL, thereisnothing, NULL)))
        {
            LOGD("can't create thread: %s\n", strerror(temp));
        }
    }
}

extern "C" {
	__attribute__((visibility("default")))
	void nativeForkAndSpecializePre(
			JNIEnv *env, jclass clazz, jint *_uid, jint *gid, jintArray *gids, jint *runtime_flags,
			jobjectArray *rlimits, jint *_mount_external, jstring *se_info, jstring *se_name,
			jintArray *fdsToClose, jintArray *fdsToIgnore, jboolean *is_child_zygote,
			jstring *instructionSet, jstring *appDataDir, jstring *packageName,
			jobjectArray *packagesForUID, jstring *sandboxId) {
		enable_hook = is_app_need_hook(env, *appDataDir);
	}

	__attribute__((visibility("default")))
	int nativeForkAndSpecializePost(JNIEnv *env, jclass clazz, jint res) {
		nativeForkAndSpecialize(res, enable_hook);
		return !enable_hook;
	}
}