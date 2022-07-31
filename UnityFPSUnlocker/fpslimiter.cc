#include "fpslimiter.hh"

#include <chrono>
#include <string>
#include <sys/mman.h>
#include <thread>
#include <unistd.h>

#include <logger.hh>
#include <xdl.h>

#define __uintval(p) reinterpret_cast<intptr_t>(p)
#define __page_size 4096
#define __page_align(n) __align_up(static_cast<intptr_t>(n), __page_size)
#define __align_up(x, n) (((x) + ((n)-1)) & ~((n)-1))
#define __align_down(x, n) ((x) & -(n))
#define __ptr(p) reinterpret_cast<void*>(p)
#define __ptr_align(x) __ptr(__align_down(reinterpret_cast<intptr_t>(x), __page_size))
#define __make_rwx(p, n)                                                                                                     \
    mprotect(__ptr_align(p),                                                                                                 \
             __page_align(__uintval(p) + n) != __page_align(__uintval(p)) ? __page_align(n) + __page_size : __page_align(n), \
             1 | 2 | 4);

#define __make_rx(p, n)                                                                                                      \
    mprotect(__ptr_align(p),                                                                                                 \
             __page_align(__uintval(p) + n) != __page_align(__uintval(p)) ? __page_align(n) + __page_size : __page_align(n), \
             1 | 4);

namespace FPSLimiter {
    static const char* il2cpp_resolve_icall_name{ "il2cpp_resolve_icall" };
    using il2cpp_resolve_icall_f = void* (*)(const char*);
    using set_targetFrameRate_f = void (*)(int);
    static il2cpp_resolve_icall_f il2cpp_resolve_icall = nullptr;
    static set_targetFrameRate_f set_targetFrameRate = nullptr;

    void Start(int delay, int framerate, bool modify_opcode) {
        logger("***** new thread *****");
        logger("delay: %d | framerate: %d | modify_opcode: %d", delay, framerate, modify_opcode);
        sleep(delay);
        logger("***** begin *****");
        void* handle = xdl_open("libil2cpp.so", 0);
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
                    int code = __make_rwx(ptr, 4);
                    if (code) {
                        logger("Change permission failed, %d %s", code, strerror(errno));
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
#endif
                    code = __make_rx(ptr, 4);
                    if (code) {
                        logger("Change permission failed, %d %s", code, strerror(errno));
                        goto FAILED;
                    }
                }

                logger("set_targetFrameRate: %d", framerate);
                logger("***** finished *****");
                return;
            }
        }
    FAILED:
        logger("Failed, not support this game.");
    }
} // namespace FPSLimiter
