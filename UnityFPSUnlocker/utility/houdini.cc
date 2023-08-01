#include "houdini.hh"

#if defined(__i386__) || defined(__x86_64__)

#include <absl/status/status.h>
#include <dlfcn.h>

#include "native_bridge_700r36.h"
#include "native_bridge_master.h"
#include "utility/native_bridge_700r36.h"
#include "utility/native_bridge_master.h"

#ifdef __x86_64__
#define syslib "/system/lib64/"
#elif defined(__i386__)
#define syslib "/system/lib/"
#endif

namespace {
    using NativeBridgeGetVersion_t = uint32_t (*)();
    using NativeBridgeLoadLibrary_t = void* (*)(const char*, int);
    using NativeBridgeLoadLibraryExt_t = void* (*)(const char*, int, void*);
    using NativeBridgeGetTrampoline_t = void* (*)(void* handle, const char* name, const char* shorty, uint32_t len);
    using NativeBridgeGetError_t = const char* (*)();

    static NativeBridgeGetVersion_t NativeBridgeGetVersion = nullptr;
    static NativeBridgeLoadLibrary_t NativeBridgeLoadLibrary = nullptr;
    static NativeBridgeLoadLibraryExt_t NativeBridgeLoadLibraryExt = nullptr;
    static NativeBridgeGetTrampoline_t NativeBridgeGetTrampoline = nullptr;
    static NativeBridgeGetError_t NativeBridgeGetError = nullptr;
} // namespace

Houdini::Houdini() {
    void* libhoudini = dlopen("libhoudini.so", RTLD_NOW);
    if (libhoudini) {
        houdini_itf_ = dlsym(libhoudini, "NativeBridgeItf");
        if (houdini_itf_) {
            auto master_ptr = reinterpret_cast<android::NativeBridgeCallbacksMaster*>(houdini_itf_);
            houdini_ver_ = master_ptr->version;
        }
    }
    else {
        void* libnativebridge = dlopen(syslib "libnativebridge.so", RTLD_NOW);
        if (libnativebridge) {
            NativeBridgeGetVersion = (NativeBridgeGetVersion_t)dlsym(libnativebridge, "_ZN7android22NativeBridgeGetVersionEv");
            houdini_ver_ = NativeBridgeGetVersion();
            NativeBridgeLoadLibrary = (NativeBridgeLoadLibrary_t)dlsym(libnativebridge, "_ZN7android23NativeBridgeLoadLibraryEPKci");
            NativeBridgeLoadLibraryExt = (NativeBridgeLoadLibraryExt_t)dlsym(libnativebridge, "_ZN7android26NativeBridgeLoadLibraryExtEPKciPNS_25native_bridge_namespace_tE");
            NativeBridgeGetTrampoline = (NativeBridgeGetTrampoline_t)dlsym(libnativebridge, "_ZN7android25NativeBridgeGetTrampolineEPvPKcS2_j");

            if (houdini_ver_ > 2) {
                NativeBridgeGetError = (NativeBridgeGetError_t)dlsym(libnativebridge, "_ZN7android20NativeBridgeGetErrorEv");
            }
        }
    }
}

absl::StatusOr<void*> Houdini::LoadLibrary(const char* name, int flag) {
    if (houdini_ver_ > 0) {
        if (houdini_itf_) {
            if (houdini_ver_ == 2) {
                auto ptr = reinterpret_cast<android::NativeBridgeCallbacks700R36*>(houdini_itf_);
                return ptr->loadLibrary(name, flag);
            }
            else {
                auto ptr = reinterpret_cast<android::NativeBridgeCallbacksMaster*>(houdini_itf_);
                return ptr->loadLibraryExt(name, flag, (void*)houdini_ver_);
            }
        }
        else {
            if (houdini_ver_ == 2) {
                return NativeBridgeLoadLibrary(name, flag);
            }
            else {
                return NativeBridgeLoadLibraryExt(name, flag, (void*)houdini_ver_);
            }
        }
    }
    return absl::InternalError("Houdini init error");
}

absl::Status Houdini::CallJNI(void* handle, void* vm, void* reserved) {
    using JNI_OnLoad_t = int (*)(void*, void*);
    JNI_OnLoad_t jni_onload_ptr = nullptr;
    if (houdini_ver_ > 0) {
        if (houdini_itf_) {
            auto ptr = reinterpret_cast<android::NativeBridgeCallbacks700R36*>(houdini_itf_);
            jni_onload_ptr = reinterpret_cast<JNI_OnLoad_t>(ptr->getTrampoline(handle, "JNI_OnLoad", nullptr, 0));
        }
        else {
            jni_onload_ptr = reinterpret_cast<JNI_OnLoad_t>(NativeBridgeGetTrampoline(handle, "JNI_OnLoad", nullptr, 0));
        }

        if (jni_onload_ptr) {
            jni_onload_ptr(vm, reserved);
            return absl::OkStatus();
        }
        else {
            return absl::NotFoundError("JNI_OnLoad not found");
        }
    }
    return absl::InternalError("Houdini init error");
}

const char* Houdini::GetError() {
    if (houdini_ver_ > 2) {
        if (houdini_itf_) {
            auto ptr = reinterpret_cast<android::NativeBridgeCallbacksMaster*>(houdini_itf_);
            return ptr->getError();
        }
        else {
            return NativeBridgeGetError();
        }
    }
    return "(null)";
}

#endif // architecture defined