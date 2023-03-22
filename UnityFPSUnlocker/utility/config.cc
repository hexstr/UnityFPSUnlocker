#include "config.hh"

#include <fstream>
#include <jni.h>
#include <string>

#include "logger.hh"

using absl::StatusOr;
using namespace rapidjson;

namespace Utility {
    StatusOr<Document> LoadJsonFromFile(const char* FilePath) {
        Document doc;
        std::ifstream file(FilePath);
        if (file.is_open()) {
            std::string content((std::istreambuf_iterator<char>(file)),
                                (std::istreambuf_iterator<char>()));
            if (doc.Parse(content.c_str()).HasParseError()) {
                if (content.size() > 2) {
                    content[doc.GetErrorOffset() - 2] = '-';
                    content[doc.GetErrorOffset() - 1] = '>';
                }
                LOG("[%s] %s #%d\nFile: %s %s %zu %s", __FUNCTION__, __FILE__, __LINE__,
                    FilePath, GetParseError_En(doc.GetParseError()), doc.GetErrorOffset(), content.c_str());
                return absl::InternalError("Parse error");
            }
        }
        else {
            LOG("[%s] %s #%d\nCould not read file: %s.", __FUNCTION__, __FILE__, __LINE__, FilePath);
            return absl::InternalError("Could not read file.");
        }
        return doc;
    }

    jobject GetApplication(JNIEnv* env) {
        jclass activity_thread_clz = env->FindClass("android/app/ActivityThread");
        if (activity_thread_clz != nullptr) {
            jmethodID currentApplicationId = env->GetStaticMethodID(
                activity_thread_clz,
                "currentApplication",
                "()Landroid/app/Application;");
            if (currentApplicationId != nullptr) {
                return env->CallStaticObjectMethod(activity_thread_clz, currentApplicationId);
            }
            else {
                ERROR("Cannot find method: currentApplication() in ActivityThread.");
            }
        }
        else {
            ERROR("Cannot find class: android.app.ActivityThread");
        }
        return nullptr;
    }

    jobject GetApplicationInfo(JNIEnv* env) {
        jobject application = GetApplication(env);
        jclass application_clazz = env->GetObjectClass(application);
        jmethodID get_application_info = env->GetMethodID(
            application_clazz,
            "getApplicationInfo",
            "()Landroid/content/pm/ApplicationInfo;");
        if (get_application_info) {
            return env->CallObjectMethod(application, get_application_info);
        }
        else {
            ERROR("No method id getApplicationInfo");
            return nullptr;
        }
    }

    std::string GetLibraryPath(JNIEnv* env, jobject application_info) {
        jfieldID native_library_dir_id = env->GetFieldID(
            env->GetObjectClass(application_info),
            "nativeLibraryDir",
            "Ljava/lang/String;");

        if (native_library_dir_id) {
            jstring native_library_dir_jstring = (jstring)env->GetObjectField(application_info, native_library_dir_id);
            const char* path = env->GetStringUTFChars(native_library_dir_jstring, 0);
            std::string package_name(path);
            env->ReleaseStringUTFChars(native_library_dir_jstring, path);
            return package_name;
        }
        else {
            ERROR("No nativeLibraryDir");
            return {};
        }
    }
} // namespace Utility