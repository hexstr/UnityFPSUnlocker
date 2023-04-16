#include "fpslimiter.hh"

#include <chrono>
#include <string>
#include <thread>

#include <xdl.h>

#include "utility/config.hh"
#include "utility/logger.hh"

namespace FPSLimiter {
    static const char* il2cpp_resolve_icall_name{ "il2cpp_resolve_icall" };
    using il2cpp_resolve_icall_f = void* (*)(const char*);
    using set_targetFrameRate_f = void (*)(int);
    static il2cpp_resolve_icall_f il2cpp_resolve_icall = nullptr;
    static set_targetFrameRate_f set_targetFrameRate = nullptr;

    void Start(int delay, int framerate, bool modify_opcode) {
#ifdef __aarch64__
        LOG("[UnityFPSUnlocker][arm64] Starting...");
#elif defined(__ARM_ARCH_7A__)
        LOG("[UnityFPSUnlocker][armv7] Starting...");
#elif defined(__i386__)
        LOG("[UnityFPSUnlocker][x86] Starting...");
#elif defined(__x86_64__)
        LOG("[UnityFPSUnlocker][x86_64] Starting...");
#endif
        LOG("delay: %d | framerate: %d | modify_opcode: %d", delay, framerate, modify_opcode);
        std::chrono::seconds sleep_duration(delay);
        std::this_thread::sleep_for(sleep_duration);
        LOG("***** begin *****");
        void* handle = nullptr;
        for (int i = 0; i < 10; ++i) {
            if ((handle = xdl_open("libil2cpp.so", 0))) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        if (handle) {
            il2cpp_resolve_icall = (il2cpp_resolve_icall_f)xdl_sym(handle, il2cpp_resolve_icall_name, nullptr);
            if (!il2cpp_resolve_icall) {
                goto FAILED;
            }

            set_targetFrameRate = (set_targetFrameRate_f)il2cpp_resolve_icall("UnityEngine.Application::set_targetFrameRate(System.Int32)");
            if (set_targetFrameRate) {
                set_targetFrameRate(framerate);

                if (modify_opcode) {
                    unsigned char* ptr = (unsigned char*)set_targetFrameRate;
                    int code = Utility::ChangeMemPermission(ptr, 4);
                    if (code) {
                        LOG("Change permission failed, %d %s", code, strerror(errno));
                        goto FAILED;
                    }
#ifdef __aarch64__
                    ptr[0] = 0xC0;
                    ptr[1] = 0x03;
                    ptr[2] = 0x5F;
                    ptr[3] = 0xD6;
#elif __ARM_ARCH_7A__
                    ptr[0] = 0x1E;
                    ptr[1] = 0xFF;
                    ptr[2] = 0x2F;
                    ptr[3] = 0xE1;
#elif defined(__i386__) || defined(__x86_64__)
                    ptr[0] = 0xC3;
#endif
                    code = Utility::ChangeMemPermission(ptr, 4);
                    if (code) {
                        LOG("Change permission failed, %d %s", code, strerror(errno));
                        goto FAILED;
                    }
                }

                LOG("set_targetFrameRate: %d", framerate);
                LOG("***** finished *****");
                return;
            }
        }
    FAILED:
        LOG("Failed, libil2cpp.so not found.");
    }
} // namespace FPSLimiter
