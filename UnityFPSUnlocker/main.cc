#include "main.hh"

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <jni.h>

#include <thread>

#include <absl/container/flat_hash_set.h>
#include <absl/status/statusor.h>

#include "config.hh"
#include "file_watch/dispatcher/epoller.hh"
#include "file_watch/listener.hh"
#include "fpslimiter.hh"
#include <logger.hh>
#include <socket.hh>

using namespace rapidjson;

static bool is_loaded = false;
static int frame_rate = 60;
static int delay = 30;
static int watch_descriptor = -1;
static absl::flat_hash_set<std::string> normal_list;
static absl::flat_hash_set<std::string> mod_list;

void LoadConfig() {
    frame_rate = 60;
    delay = 30;
    normal_list.clear();
    mod_list.clear();

    auto read_path = Utility::LoadJsonFromFile("/data/local/tmp/TargetList.json");
    if (!read_path.ok()) {
        return;
    }
    Document& doc = *read_path;
    if (auto itor = doc.FindMember("normal_list"); itor != doc.MemberEnd() && itor->value.IsArray()) {
        for (const auto& package : itor->value.GetArray()) {
            normal_list.emplace(package.GetString());
        }
    }
    if (auto itor = doc.FindMember("mod_list"); itor != doc.MemberEnd() && itor->value.IsArray()) {
        for (const auto& package : itor->value.GetArray()) {
            mod_list.emplace(package.GetString());
        }
    }
    if (auto itor = doc.FindMember("framerate"); itor != doc.MemberEnd() && itor->value.IsInt()) {
        frame_rate = itor->value.GetInt();
    }
    if (auto itor = doc.FindMember("delay"); itor != doc.MemberEnd() && itor->value.IsInt()) {
        delay = itor->value.GetInt();
    }

    logger("[LoadConfig] normal_list: %zu, mod_list: %zu frame_rate: %d delay: %d", normal_list.size(), mod_list.size(), frame_rate, delay);
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
        logger("[UnityFPSUnlocker]initializing...");
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

    if (normal_list.contains(package_name)) {
        write_int(s, 1);
        write_int(s, delay);
        write_int(s, frame_rate);
        write_int(s, false);
    } else if (mod_list.contains(package_name)) {
        write_int(s, 1);
        write_int(s, delay);
        write_int(s, frame_rate);
        write_int(s, true);
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