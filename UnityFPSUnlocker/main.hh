#ifndef MAIN_HEADER
#define MAIN_HEADER

#include "third/zygisk.hh"

#include "logger.hh"

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
    int has_custom_cfg_ = false;
    int delay_ = 5;
    int framerate_ = 60;
    bool modify_opcode_ = false;
    const char* package_name_ = nullptr;

    void preSpecialize(const char* process);
};

class ConfigValue {
public:
    int delay = 7;
    int fps = 77;
    bool mod_opcode = false;

    ConfigValue(){};
    ConfigValue(const ConfigValue& lhs) {
        delay = lhs.delay;
        fps = lhs.fps;
        mod_opcode = lhs.mod_opcode;
    }

    void DebugPrint() {
        LOG("\tdelay: %d | fps: %d | mod_opcode: %d", delay, fps, mod_opcode);
    }
};

#endif // main.hh