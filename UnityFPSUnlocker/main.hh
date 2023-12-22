#ifndef MAIN_HEADER
#define MAIN_HEADER

#include "third/zygisk.hh"

#include "utility/config.hh"
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
    const char* package_name_ = nullptr;
    ConfigValue current_cfg_;

    void ForHoudini();
};

#endif // main.hh