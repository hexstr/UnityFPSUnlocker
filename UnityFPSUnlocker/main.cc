#include "main.hh"

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <jni.h>

#include <thread>

#include <absl/container/flat_hash_map.h>
#include <absl/status/statusor.h>

#include "config.hh"
#include "file_watch/dispatcher/epoller.hh"
#include "file_watch/listener.hh"
#include "fpslimiter.hh"
#include <logger.hh>
#include <socket.hh>

using namespace rapidjson;

static bool is_loaded = false;
static int watch_descriptor = -1;
static absl::flat_hash_map<std::string, ConfigValue> custom_list;
static ConfigValue global_cfg;

void LoadConfig() {
    custom_list.clear();

    auto read_path = Utility::LoadJsonFromFile("/data/local/tmp/TargetList.json");
    if (!read_path.ok()) {
        return;
    }

    Document& doc = *read_path;
    if (auto itor = doc.FindMember("global"); itor != doc.MemberEnd() && itor->value.IsObject()) {
        if (auto itor2 = itor->value.FindMember("delay"); itor2 != doc.MemberEnd() && itor2->value.IsInt()) {
            global_cfg.delay = itor2->value.GetInt();
        }
        if (auto itor2 = itor->value.FindMember("fps"); itor2 != doc.MemberEnd() && itor2->value.IsInt()) {
            global_cfg.fps = itor2->value.GetInt();
        }
        if (auto itor2 = itor->value.FindMember("mod_opcode"); itor2 != doc.MemberEnd() && itor2->value.IsBool()) {
            global_cfg.mod_opcode = itor2->value.GetBool();
        }
    }

    if (auto itor = doc.FindMember("custom"); itor != doc.MemberEnd() && itor->value.IsObject()) {
        for (auto&& item : itor->value.GetObject()) {
            if (item.value.IsObject()) {
                auto cfg(global_cfg);
                if (item.value.MemberCount()) {
                    if (auto itor2 = item.value.FindMember("delay"); itor2 != item.value.MemberEnd() && itor2->value.IsInt()) {
                        cfg.delay = itor2->value.GetInt();
                    }
                    if (auto itor2 = item.value.FindMember("fps"); itor2 != item.value.MemberEnd() && itor2->value.IsInt()) {
                        cfg.fps = itor2->value.GetInt();
                    }
                    if (auto itor2 = item.value.FindMember("mod_opcode"); itor2 != item.value.MemberEnd() && itor2->value.IsBool()) {
                        cfg.mod_opcode = itor2->value.GetBool();
                    }
                }
                custom_list[item.name.GetString()] = cfg;
            }
        }
    }

    logger("[LoadConfig] custom_list: %zu", custom_list.size());
    logger("[LoadConfig] global_cfg: ");
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
#ifdef __aarch64__
        logger("[UnityFPSUnlocker][arm64] Initializing...");
#elif defined(__ARM_ARCH_7A__)
        logger("[UnityFPSUnlocker][armv7] Initializing...");
#elif defined(__i386__)
        logger("[UnityFPSUnlocker][x86] Initializing...");
#elif defined(__x86_64__)
        logger("[UnityFPSUnlocker][x86_64] Initializing...");
#endif
        LoadConfig();
        EPoller* file_watch_poller = new EPoller(new FileWatch::Listener());
        EPoller::reserved_list_.push_back(file_watch_poller);
        std::thread([=] {
            while (true) {
                file_watch_poller->Poll();
            }
        }).detach();
        watch_descriptor = FileWatch::Listener::Register("/data/local/tmp/TargetList.json", OnModified);
        is_loaded = true;
    }

    if (auto itor = custom_list.find(package_name); itor != custom_list.end()) {
        write_int(s, 1);
        write_int(s, itor->second.delay);
        write_int(s, itor->second.fps);
        write_int(s, itor->second.mod_opcode);
    } else {
        write_int(s, 0); // is_target : false
    }
}

REGISTER_ZYGISK_MODULE(MyModule)
REGISTER_ZYGISK_COMPANION(CompanionEntry)

void MyModule::onLoad(Api* api, JNIEnv* env) {
    this->api = api;
    this->env = env;
}

void MyModule::preAppSpecialize(AppSpecializeArgs* args) {
    const char* process = env->GetStringUTFChars(args->nice_name, nullptr);
    preSpecialize(process);
    env->ReleaseStringUTFChars(args->nice_name, process);
}

void MyModule::postAppSpecialize(const AppSpecializeArgs* args) {
    if (is_target_) {
        std::thread([=]() {
            FPSLimiter::Start(delay_, framerate_, modify_opcode_);
        }).detach();
    }
}

void MyModule::preSpecialize(const char* process) {
    int client_socket = api->connectCompanion();
    write_string(client_socket, process);
    is_target_ = read_int(client_socket); // is_target

    if (is_target_ == 1) {
        delay_ = read_int(client_socket);
        framerate_ = read_int(client_socket);
        modify_opcode_ = read_int(client_socket);
    }

    close(client_socket);
}