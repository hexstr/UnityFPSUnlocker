#ifndef MAIN_HEADER
#define MAIN_HEADER

#include "third/zygisk.hh"

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
    int is_target_ = false;
    int delay_ = 5;
    int framerate_ = 60;
    bool modify_opcode_ = false;

    void preSpecialize(const char* process);
};

#endif // main.hh