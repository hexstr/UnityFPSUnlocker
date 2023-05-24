#include "main.hh"

#include <dlfcn.h>
#include <fcntl.h>
#include <jni.h>

#include <chrono>
#include <fstream>
#include <thread>

#include <absl/container/flat_hash_map.h>
#include <absl/status/status.h>
#include <absl/status/statusor.h>
#include <absl/strings/str_split.h>
#include <absl/strings/substitute.h>

#include "file_watch/dispatcher/epoller.hh"
#include "file_watch/listener.hh"
#include "fpslimiter.hh"
#include "third/riru_hide/hide.hh"
#include "utility/config.hh"
#include "utility/houdini.hh"
#include "utility/socket.hh"

using namespace rapidjson;

static bool is_loaded = false;
static int watch_descriptor = -1;
static absl::flat_hash_map<std::string, ConfigValue> custom_list;
static ConfigValue global_cfg;
static FileWatch::Listener* file_watch_listener = nullptr;

constexpr const char* ConfigFile = "/data/local/tmp/TargetList.json";
constexpr const char* ErrorFile = "/data/local/tmp/gh@hexstr/error.log";

absl::Status LoadConfig() {
    custom_list.clear();

    auto read_path = Utility::LoadJsonFromFile(ConfigFile);
    if (!read_path.ok()) {
        return read_path.status();
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

    return absl::OkStatus();
}

void OnModified(int wd) {
    if (wd == watch_descriptor) {
        LoadConfig().IgnoreError();
    }
}

void OnDeleted() {
    watch_descriptor = -1;
}

// In zygiskd memory.
void CompanionEntry(int s) {
    std::string package_name = read_string(s);
    if (is_loaded == false) {
        if (auto res = LoadConfig(); res.ok()) {
            is_loaded = true;
            file_watch_listener = new FileWatch::Listener();
            EPoller* file_watch_poller = new EPoller(file_watch_listener);
            EPoller::reserved_list_.push_back(file_watch_poller);
            std::thread([=] {
                while (true) {
                    file_watch_poller->Poll();
                }
            }).detach();
            watch_descriptor = file_watch_listener->Register(ConfigFile, OnModified, OnDeleted);
        }
        else {
            ERROR("LoadConfig error: %s", res.message().data());
        }
    }

    if (is_loaded && watch_descriptor == -1) {
        watch_descriptor = file_watch_listener->Register(ConfigFile, OnModified, OnDeleted);
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
    int client_socket = api->connectCompanion();
    write_string(client_socket, package_name_);

    has_custom_cfg_ = read_int(client_socket);
    delay_ = read_int(client_socket);
    framerate_ = read_int(client_socket);
    modify_opcode_ = read_int(client_socket);

    close(client_socket);
}

void MyModule::ForHoudini() {
#if defined(__i386__) || defined(__x86_64__)
    std::thread([=]() {
        std::chrono::seconds sleep_duration(delay_);
        std::this_thread::sleep_for(sleep_duration);
#ifdef __x86_64__
#define syslib "/system/lib64/"
#define libdir "/lib/x86_64"
#define library_name "arm64-v8a.so"
#endif

#ifdef __i386__
#define syslib "/system/lib/"
#define libdir "/lib/x86"
#define library_name "armeabi-v7a.so"
#endif

        auto vms = Utility::GetVM(syslib "libart.so");
        if (!vms.ok()) {
            ERROR("%s", vms.status().message().data());
            return;
        }

        JNIEnv* env = nullptr;
        if (vms.value()->AttachCurrentThread(&env, nullptr) < 0) {
            ERROR("Cannot connect to JNI environment");
            return;
        }

        auto app_info = Utility::GetApplicationInfo(env);
        auto path = Utility::GetLibraryPath(env, app_info.value());

        if (!path.ok()) {
            ERROR("%s", vms.status().message().data());
            return;
        }

        if (path.value().find(libdir) == std::string::npos) {
            auto& houdini = Houdini::GetInstance();
            auto plugin = houdini.LoadLibrary("/data/local/tmp/gh@hexstr/UnityFPSUnlocker/" library_name, RTLD_NOW);
            if (plugin.ok()) {
                ConfigValue config(0, framerate_, modify_opcode_);
                if (auto result = houdini.CallJNI(plugin.value(), vms.value(), &config);
                    !result.ok()) {
                    ERROR("%s", plugin.status().message().data());
                }
                riru_hide("/data/local/tmp/gh@hexstr/UnityFPSUnlocker/" library_name);
            }
            else {
                ERROR("%s", plugin.status().message().data());
            }
        }
        else {
            FPSLimiter::Start(delay_, framerate_, modify_opcode_);
        }
    }).detach();
#endif
}

void MyModule::postAppSpecialize(const AppSpecializeArgs* args) {
    auto path = absl::Substitute("/sdcard/Android/data/$0/files/il2cpp", package_name_);
    if (has_custom_cfg_ || access(path.c_str(), F_OK) == 0) {
#if defined(__ARM_ARCH_7A__) || defined(__aarch64__)
        std::thread([=]() {
            FPSLimiter::Start(delay_, framerate_, modify_opcode_);
        }).detach();
#endif
        ForHoudini();
    }
    env->ReleaseStringUTFChars(args->nice_name, package_name_);
}

#if defined(__ARM_ARCH_7A__) || defined(__aarch64__)

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    int delay = 5;
    int fps = 90;
    int mod_opcode = true;

    if (reserved) {
        ConfigValue* config = reinterpret_cast<ConfigValue*>(reserved);
        delay = config->delay_;
        fps = config->fps_;
        mod_opcode = config->mod_opcode_;
    }
    else {
        std::ifstream file("/proc/self/cmdline");
        std::string cmdline;
        std::getline(file, cmdline, '\0');
        file.close();

        LOG("cmdline: %s", cmdline.c_str());

        auto path = absl::Substitute("/sdcard/Android/data/$0/files/config", cmdline);

        std::ifstream config(path);
        std::getline(config, cmdline, '\0');
        config.close();

        std::vector<std::string> v = absl::StrSplit(cmdline, '|');
        if (v.size() == 3) {
            delay = std::stoi(v[0]);
            fps = std::stoi(v[1]);
            mod_opcode = std::stoi(v[2]);
        }
    }

    std::thread([=]() {
        FPSLimiter::Start(delay, fps, mod_opcode);
    }).detach();
    return JNI_VERSION_1_6;
}

#endif