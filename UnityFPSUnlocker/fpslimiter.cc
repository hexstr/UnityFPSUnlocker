#include "fpslimiter.hh"

#include <chrono>
#include <string>
#include <thread>

#include "unity/unity_engine.hh"
#include "utility/logger.hh"

#include <xdl.h>

namespace FPSLimiter {
    void Start(const ConfigValue& cfg) {
#ifdef __aarch64__
        LOG("[UnityFPSUnlocker][arm64] Starting...");
#elif defined(__ARM_ARCH_7A__)
        LOG("[UnityFPSUnlocker][armv7] Starting...");
#elif defined(__i386__)
        LOG("[UnityFPSUnlocker][x86] Starting...");
#elif defined(__x86_64__)
        LOG("[UnityFPSUnlocker][x86_64] Starting...");
#endif
        cfg.DebugPrint();
        std::chrono::seconds sleep_duration(cfg.delay_);
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
            auto result = Unity::GetInstance().Init(handle);
            if (!result.ok()) {
                ERROR("%s", result.message().data());
            }

            Unity::GetInstance().SetFrameRate(cfg.fps_, cfg.mod_opcode_);
            Unity::GetInstance().SetResolution(cfg.scale_);

            LOG("***** finished *****");
            return;
        }
        else {
            ERROR("Failed to open libil2cpp.so");
        }
    }
} // namespace FPSLimiter
