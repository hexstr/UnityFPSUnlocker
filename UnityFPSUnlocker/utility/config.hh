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
    jobject GetApplicationInfo(JNIEnv* env);
    std::string GetLibraryPath(JNIEnv* env, jobject application_info);
}; // namespace Utility

#endif // config.hh