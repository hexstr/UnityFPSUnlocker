#include "jni_helper.hh"

namespace JNIHelper {
    jobject GetDisplay(JNIEnv* env, jobject activity) {
        jclass activity_class = env->GetObjectClass(activity);
        JNIEX(env, activity_class);

        jclass build_version_class = env->FindClass("android/os/Build$VERSION");
        JNIEX(env, build_version_class);

        jfieldID sdk_int_field = env->GetStaticFieldID(build_version_class, "SDK_INT", "I");
        jint sdk_int = env->GetStaticIntField(build_version_class, sdk_int_field);

        if (sdk_int >= 30) {
            jmethodID get_display_method = env->GetMethodID(activity_class, "getDisplay", "()Landroid/view/Display;");
            JNIEX(env, get_display_method);
            return env->CallObjectMethod(activity, get_display_method);
        }
        else {
            jclass context_class = env->FindClass("android/content/Context");
            JNIEX(env, context_class);
            jfieldID window_service_field = env->GetStaticFieldID(context_class, "WINDOW_SERVICE", "Ljava/lang/String;");
            jobject window_service_string = env->GetStaticObjectField(context_class, window_service_field);

            jmethodID get_system_service = env->GetMethodID(activity_class, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
            JNIEX(env, get_system_service);
            jobject window_manager = env->CallObjectMethod(activity, get_system_service);

            jclass window_manager_class = env->GetObjectClass(window_manager);
            JNIEX(env, window_manager_class);
            jmethodID get_default_display_method = env->GetMethodID(window_manager_class, "getDefaultDisplay", "()Landroid/view/Display;");
            JNIEX(env, get_default_display_method);
            return env->CallObjectMethod(window_manager, get_default_display_method);
        }
    }

    jobject GetActivity(JNIEnv* env) {
        jclass unity_player_class = env->FindClass("com/unity3d/player/UnityPlayer");
        JNIEX(env, unity_player_class);

        jfieldID current_activity_field = env->GetStaticFieldID(unity_player_class, "currentActivity", "Landroid/app/Activity;");

        jobject current_activity = env->GetStaticObjectField(unity_player_class, current_activity_field);
        JNIEX(env, current_activity);

        return current_activity;
    }

    jint GetPreferredModeId(JNIEnv* env, jobject activity) {
        jclass activity_class = env->GetObjectClass(activity);
        jmethodID get_window_method = env->GetMethodID(activity_class, "getWindow", "()Landroid/view/Window;");
        jobject window = env->CallObjectMethod(activity, get_window_method);

        jclass window_class = env->GetObjectClass(window);
        jmethodID get_attributes_method = env->GetMethodID(window_class, "getAttributes", "()Landroid/view/WindowManager$LayoutParams;");
        jobject params = env->CallObjectMethod(window, get_attributes_method);

        jclass layout_params_class = env->GetObjectClass(params);
        jfieldID preferred_mode_id_field = env->GetFieldID(layout_params_class, "preferredDisplayModeId", "I");
        jint preferred_mode_id = env->GetIntField(params, preferred_mode_id_field);

        return preferred_mode_id;
    }

    std::vector<Mode> GetSupportedModes(JNIEnv* env, jobject activity) {
        std::vector<Mode> ret;

        if (jobject display = GetDisplay(env, activity)) {
            jclass display_class = env->GetObjectClass(display);
            jmethodID get_supported_modes_method = env->GetMethodID(display_class, "getSupportedModes", "()[Landroid/view/Display$Mode;");
            jobjectArray modes_array = (jobjectArray)env->CallObjectMethod(display, get_supported_modes_method);

            jsize length = env->GetArrayLength(modes_array);

            jclass mode_class = env->FindClass("android/view/Display$Mode");
            jmethodID get_mode_id_method = env->GetMethodID(mode_class, "getModeId", "()I");
            jmethodID get_physical_width_method = env->GetMethodID(mode_class, "getPhysicalWidth", "()I");
            jmethodID get_physical_height_method = env->GetMethodID(mode_class, "getPhysicalHeight", "()I");
            jmethodID get_refresh_rate_method = env->GetMethodID(mode_class, "getRefreshRate", "()F");

            for (jsize i = 0; i < length; i++) {
                jobject mode_obj = env->GetObjectArrayElement(modes_array, i);
                Mode mode;
                mode.id = env->CallIntMethod(mode_obj, get_mode_id_method);
                mode.width = env->CallIntMethod(mode_obj, get_physical_width_method);
                mode.height = env->CallIntMethod(mode_obj, get_physical_height_method);
                mode.refresh_rate = env->CallFloatMethod(mode_obj, get_refresh_rate_method);
                ret.emplace_back(mode);
                env->DeleteLocalRef(mode_obj);
            }
        }

        return ret;
    }

    void SetPreferredMode(JNIEnv* env, jobject activity, jint mode) {
        jclass activity_class = env->GetObjectClass(activity);
        jmethodID get_window_method = env->GetMethodID(activity_class, "getWindow", "()Landroid/view/Window;");
        jobject window = env->CallObjectMethod(activity, get_window_method);

        jclass window_class = env->GetObjectClass(window);
        jmethodID get_attributes_method = env->GetMethodID(window_class, "getAttributes", "()Landroid/view/WindowManager$LayoutParams;");
        jobject params = env->CallObjectMethod(window, get_attributes_method);

        jclass layout_params_class = env->GetObjectClass(params);
        jfieldID preferred_mode_id_field = env->GetFieldID(layout_params_class, "preferredDisplayModeId", "I");
        env->SetIntField(params, preferred_mode_id_field, mode);

        jmethodID set_attributes_method = env->GetMethodID(window_class, "setAttributes", "(Landroid/view/WindowManager$LayoutParams;)V");
        env->CallVoidMethod(window, set_attributes_method, params);
    }

    void JNIExceptionHandler(JNIEnv* env) {
        jthrowable exc = env->ExceptionOccurred();
        if (exc) {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
    }

} // namespace JNIHelper