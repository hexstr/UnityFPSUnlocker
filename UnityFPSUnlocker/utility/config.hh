#ifndef CONFIG_HEADER
#define CONFIG_HEADER

#include <jni.h>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include <absl/status/status.h>
#include <absl/status/statusor.h>

namespace Utility {
    absl::StatusOr<rapidjson::Document> LoadJsonFromFile(const char*);
    jobject GetApplication(JNIEnv* env);
    absl::StatusOr<jobject> GetApplicationInfo(JNIEnv* env);
    absl::StatusOr<std::string> GetLibraryPath(JNIEnv* env, jobject application_info);
    absl::StatusOr<JavaVM*> GetVM(const char* art_lib);
}; // namespace Utility

#endif // config.hh