#include "main.hh"

#include <cstring>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <jni.h>

#include <thread>

#include <absl/container/flat_hash_map.h>
#include <absl/status/statusor.h>

#include "file_watch/dispatcher/epoller.hh"
#include "file_watch/listener.hh"
#include "fpslimiter.hh"
#include "utility/config.hh"
#include "utility/socket.hh"

using namespace rapidjson;

static std::atomic<bool> is_loaded = false;
static int watch_descriptor = -1;
static absl::flat_hash_map<std::string, ConfigValue> custom_list;
static ConfigValue global_cfg;

constexpr const char* ConfigFile = "/data/local/tmp/TargetList.json";

void LoadConfig() {
    custom_list.clear();

    auto read_path = Utility::LoadJsonFromFile(ConfigFile);
    if (!read_path.ok()) {
        return;
    }

    Document& doc = *read_path;
    if (auto itor = doc.FindMember("global"); itor != doc.MemberEnd() && itor->value.IsObject()) {
        if (auto itor2 = itor->value.FindMember("delay"); itor2 != doc.MemberEnd() && itor2->value.IsInt()) {
            global_cfg.delay_ = itor2->value.GetInt();
        }
        if (auto itor2 = itor->value.FindMember("fps"); itor2 != doc.MemberEnd() && itor2->value.IsInt()) {
            global_cfg.fps_ = itor2->value.GetInt();
        }
        if (auto itor2 = itor->value.FindMember("mod_opcode"); itor2 != doc.MemberEnd() && itor2->value.IsBool()) {
            global_cfg.mod_opcode_ = itor2->value.GetBool();
        }
    }

    if (auto itor = doc.FindMember("custom"); itor != doc.MemberEnd() && itor->value.IsObject()) {
        for (auto&& item : itor->value.GetObject()) {
            if (item.value.IsObject()) {
                auto cfg(global_cfg);
                if (item.value.MemberCount()) {
                    if (auto itor2 = item.value.FindMember("delay"); itor2 != item.value.MemberEnd() && itor2->value.IsInt()) {
                        cfg.delay_ = itor2->value.GetInt();
                    }
                    if (auto itor2 = item.value.FindMember("fps"); itor2 != item.value.MemberEnd() && itor2->value.IsInt()) {
                        cfg.fps_ = itor2->value.GetInt();
                    }
                    if (auto itor2 = item.value.FindMember("mod_opcode"); itor2 != item.value.MemberEnd() && itor2->value.IsBool()) {
                        cfg.mod_opcode_ = itor2->value.GetBool();
                    }
                }
                custom_list[item.name.GetString()] = cfg;
            }
        }
    }

    LOG("[LoadConfig] custom_list: %zu", custom_list.size());
    LOG("[LoadConfig] global_cfg: ");
    global_cfg.DebugPrint();
}

void OnModified(int wd) {
    if (wd == watch_descriptor) {
        LoadConfig();
    }
}

// In zygiskd memory.
void CompanionEntry(int s) {
    std::string package_name = read_string(s);
    if (is_loaded == false) {
        is_loaded = true;
        FileWatch::Listener* file_watch_listener = new FileWatch::Listener();
        EPoller* file_watch_poller = new EPoller(file_watch_listener);
        EPoller::reserved_list_.push_back(file_watch_poller);
        std::thread([=] {
            while (true) {
                file_watch_poller->Poll();
            }
        }).detach();
        watch_descriptor = file_watch_listener->Register(ConfigFile, OnModified);
        if (watch_descriptor != -1) {
            LoadConfig();
        }
    }

    if (auto itor = custom_list.find(package_name); itor != custom_list.end()) {
        write_int(s, 1);
        write_int(s, itor->second.delay_);
        write_int(s, itor->second.fps_);
        write_int(s, itor->second.mod_opcode_);
    }
    else {
        write_int(s, 0);
        write_int(s, global_cfg.delay_);
        write_int(s, global_cfg.fps_);
        write_int(s, global_cfg.mod_opcode_);
    }
}

REGISTER_ZYGISK_MODULE(MyModule)
REGISTER_ZYGISK_COMPANION(CompanionEntry)

void MyModule::onLoad(Api* api, JNIEnv* env) {
    this->api = api;
    this->env = env;
}

void MyModule::preAppSpecialize(AppSpecializeArgs* args) {
    package_name_ = env->GetStringUTFChars(args->nice_name, nullptr);
    preSpecialize(package_name_);
}

void MyModule::postAppSpecialize(const AppSpecializeArgs* args) {
    char buffer[512];
    std::sprintf(buffer, "/sdcard/Android/data/%s/files/il2cpp", package_name_);
    if (has_custom_cfg_ || access(buffer, F_OK) == 0) {
#if defined(__ARM_ARCH_7A__) || defined(__aarch64__)
        std::thread([=]() {
            FPSLimiter::Start(delay_, framerate_, modify_opcode_);
        }).detach();
#elif defined(__i386__) || defined(__x86_64__)
        std::thread([=]() {
            sleep(delay_);

            void *handle, *art = nullptr;
#ifdef __x86_64__
            handle = dlopen("/system/lib64/libnativebridge.so", RTLD_NOW);
            art = dlopen("/system/lib64/libart.so", RTLD_NOW);
#define library_name "arm64-v8a.so"
#endif

#ifdef __i386__
            handle = dlopen("/system/lib/libnativebridge.so", RTLD_NOW);
            art = dlopen("/system/lib/libart.so", RTLD_NOW);
#define library_name "armeabi-v7a.so"
#endif

            // copy from https://github.com/frida/frida-core/blob/main/lib/agent/agent.vala
            JavaVM* vms = nullptr;
            if (!art) {
                ERROR("Cannot open libart.so.");
                return;
            }

            using JNIGetCreatedJavaVMs_t = int (*)(JavaVM * *vmBuf, jsize bufLen, jsize * nVMs);
            static JNIGetCreatedJavaVMs_t JNIGetCreatedJavaVMsFunc = (JNIGetCreatedJavaVMs_t)dlsym(art, "JNI_GetCreatedJavaVMs");

            if (!JNIGetCreatedJavaVMsFunc) {
                ERROR("Cannot get vms");
                return;
            }

            jsize numVMs;
            if (JNIGetCreatedJavaVMsFunc(&vms, 1, &numVMs) != 0) {
                ERROR("Cannot get vms");
                return;
            }

            JNIEnv* env = nullptr;
            if (vms->AttachCurrentThread(&env, nullptr) < 0) {
                ERROR("Cannot connect to JNI environment");
                return;
            }

            jobject app_info = Utility::GetApplicationInfo(env);
            auto path = Utility::GetLibraryPath(env, app_info);

            if (path.empty()) {
                return;
            }

            if (path.find("/lib/x86") != std::string::npos) {
                FPSLimiter::Start(delay_, framerate_, modify_opcode_);
            }
            else {
                using NativeBridgeLoadLibraryExt_t = void* (*)(const char*, int, int);
                static NativeBridgeLoadLibraryExt_t NativeBridgeLoadLibraryExt = nullptr;
                NativeBridgeLoadLibraryExt = (NativeBridgeLoadLibraryExt_t)dlsym(handle, "_ZN7android26NativeBridgeLoadLibraryExtEPKciPNS_25native_bridge_namespace_tE");

                using NativeBridgeGetTrampoline_t = void* (*)(void* handle, const char* name, const char* shorty, uint32_t len);
                static NativeBridgeGetTrampoline_t NativeBridgeGetTrampoline = nullptr;
                NativeBridgeGetTrampoline = (NativeBridgeGetTrampoline_t)dlsym(handle, "_ZN7android25NativeBridgeGetTrampolineEPvPKcS2_j");

                void* result = NativeBridgeLoadLibraryExt("/data/local/tmp/gh@hexstr/UnityFPSUnlocker/" library_name, RTLD_NOW, 3);
                if (result) {
                    using JNIFunc = int (*)(void*, ConfigValue*);
                    JNIFunc jni_onload = (JNIFunc)NativeBridgeGetTrampoline(result, "JNI_OnLoad", nullptr, 0);
                    if (jni_onload) {
                        ConfigValue config(0, framerate_, modify_opcode_);
                        jni_onload(vms, &config);
                    }
                    else {
                        ERROR("Load plugin failed.");
                    }
                }
                else {
                    ERROR("Load plugin failed.");
                }
            }
        }).detach();

#endif
    }
    env->ReleaseStringUTFChars(args->nice_name, package_name_);
}

void MyModule::preSpecialize(const char* process) {
    int client_socket = api->connectCompanion();
    write_string(client_socket, process);

    has_custom_cfg_ = read_int(client_socket);
    delay_ = read_int(client_socket);
    framerate_ = read_int(client_socket);
    modify_opcode_ = read_int(client_socket);

    close(client_socket);
}

#if defined(__ARM_ARCH_7A__) || defined(__aarch64__)

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    ConfigValue* config = reinterpret_cast<ConfigValue*>(reserved);
    int delay = config->delay_;
    int fps = config->fps_;
    int mod_opcode = config->mod_opcode_;
    std::thread([=]() {
        FPSLimiter::Start(delay, fps, mod_opcode);
    }).detach();
    return JNI_VERSION_1_6;
}

#endif