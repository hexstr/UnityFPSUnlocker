#ifndef MAIN_HEADER
#define MAIN_HEADER

#include "third/zygisk.hh"

#include "utility/logger.hh"

using zygisk::Api;
using zygisk::AppSpecializeArgs;

class MyModule : public zygisk::ModuleBase {
public:
    virtual void onLoad(Api* api, JNIEnv* env) override final;
    virtual void preAppSpecialize(AppSpecializeArgs* args) override final;
    virtual void postAppSpecialize(const AppSpecializeArgs* args) override final;

private:
    Api* api;
    JNIEnv* env;
    int module_dir_fd_ = -1;
    int has_custom_cfg_ = false;
    int delay_ = 5;
    int framerate_ = 60;
    bool modify_opcode_ = false;
    const char* package_name_ = nullptr;

    void ForHoudini();
};

class ConfigValue {
public:
    int delay_ = 7;
    int fps_ = 77;
    bool mod_opcode_ = false;

    ConfigValue(){};
    ConfigValue(int delay, int fps, bool mod_opcode) : delay_(delay), fps_(fps), mod_opcode_(mod_opcode){};
    ConfigValue(const ConfigValue& lhs) {
        delay_ = lhs.delay_;
        fps_ = lhs.fps_;
        mod_opcode_ = lhs.mod_opcode_;
    }

    void DebugPrint() {
        LOG("\tdelay: %d | fps: %d | mod_opcode: %d", delay_, fps_, mod_opcode_);
    }
};

#endif // main.hh