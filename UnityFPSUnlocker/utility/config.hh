#ifndef CONFIG_HEADER
#define CONFIG_HEADER

#include <jni.h>
#include <sys/mman.h>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include <absl/status/status.h>
#include <absl/status/statusor.h>

#include "logger.hh"

class ConfigValue {
public:
    int delay_ = 5;
    int fps_ = 90;
    bool mod_opcode_ = true;
    float scale_ = -1;

    ConfigValue(){};
    ConfigValue(int delay, int fps, bool mod_opcode, float scale)
        : delay_(delay),
          fps_(fps),
          mod_opcode_(mod_opcode),
          scale_(scale){};
    ConfigValue(const ConfigValue& lhs) {
        delay_ = lhs.delay_;
        fps_ = lhs.fps_;
        mod_opcode_ = lhs.mod_opcode_;
        scale_ = lhs.scale_;
    }

    void DebugPrint() const {
        LOG("\tdelay: %d | fps: %d | mod_opcode: %d | scale: %f", delay_, fps_, mod_opcode_, scale_);
    }
};

namespace Utility {
    absl::StatusOr<rapidjson::Document> LoadJsonFromFile(const char*);
    jobject GetApplication(JNIEnv* env);
    absl::StatusOr<jobject> GetApplicationInfo(JNIEnv* env);
    absl::StatusOr<std::string> GetLibraryPath(JNIEnv* env, jobject application_info);
    absl::StatusOr<JavaVM*> GetVM(const char* art_lib);
    int ChangeMemPermission(void* p, size_t n, int permission = PROT_READ | PROT_WRITE | PROT_EXEC);
    void NopFunc(unsigned char* ptr);
}; // namespace Utility

#endif // config.hh