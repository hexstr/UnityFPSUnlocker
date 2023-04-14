/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef NATIVE_BRIDGE_H_
#define NATIVE_BRIDGE_H_

#include <signal.h>
#include <stdint.h>
#include <sys/types.h>
namespace android {
    struct NativeBridgeRuntimeCallbacks;
    struct NativeBridgeRuntimeValues;
    // Function pointer type for sigaction. This is mostly the signature of a signal handler, except
    // for the return type. The runtime needs to know whether the signal was handled or should be given
    // to the chain.
    typedef bool (*NativeBridgeSignalHandlerFn)(int, siginfo_t*, void*);

    // Native bridge interfaces to runtime.
    struct NativeBridgeCallbacks700R36 {
        // Version number of the interface.
        uint32_t version;
        // Initialize native bridge. Native bridge's internal implementation must ensure MT safety and
        // that the native bridge is initialized only once. Thus it is OK to call this interface for an
        // already initialized native bridge.
        //
        // Parameters:
        //   runtime_cbs [IN] the pointer to NativeBridgeRuntimeCallbacks.
        // Returns:
        //   true iff initialization was successful.
        bool (*initialize)(const NativeBridgeRuntimeCallbacks* runtime_cbs, const char* private_dir,
                           const char* instruction_set);
        // Load a shared library that is supported by the native bridge.
        //
        // Parameters:
        //   libpath [IN] path to the shared library
        //   flag [IN] the stardard RTLD_XXX defined in bionic dlfcn.h
        // Returns:
        //   The opaque handle of the shared library if sucessful, otherwise NULL
        void* (*loadLibrary)(const char* libpath, int flag);
        // Get a native bridge trampoline for specified native method. The trampoline has same
        // sigature as the native method.
        //
        // Parameters:
        //   handle [IN] the handle returned from loadLibrary
        //   shorty [IN] short descriptor of native method
        //   len [IN] length of shorty
        // Returns:
        //   address of trampoline if successful, otherwise NULL
        void* (*getTrampoline)(void* handle, const char* name, const char* shorty, uint32_t len);
        // Check whether native library is valid and is for an ABI that is supported by native bridge.
        //
        // Parameters:
        //   libpath [IN] path to the shared library
        // Returns:
        //   TRUE if library is supported by native bridge, FALSE otherwise
        bool (*isSupported)(const char* libpath);
        // Provide environment values required by the app running with native bridge according to the
        // instruction set.
        //
        // Parameters:
        //    instruction_set [IN] the instruction set of the app
        // Returns:
        //    NULL if not supported by native bridge.
        //    Otherwise, return all environment values to be set after fork.
        const struct NativeBridgeRuntimeValues* (*getAppEnv)(const char* instruction_set);
        // Added callbacks in version 2.
        // Check whether the bridge is compatible with the given version. A bridge may decide not to be
        // forwards- or backwards-compatible, and libnativebridge will then stop using it.
        //
        // Parameters:
        //     bridge_version [IN] the version of libnativebridge.
        // Returns:
        //     true iff the native bridge supports the given version of libnativebridge.
        bool (*isCompatibleWith)(uint32_t bridge_version);
        // A callback to retrieve a native bridge's signal handler for the specified signal. The runtime
        // will ensure that the signal handler is being called after the runtime's own handler, but before
        // all chained handlers. The native bridge should not try to install the handler by itself, as
        // that will potentially lead to cycles.
        //
        // Parameters:
        //     signal [IN] the signal for which the handler is asked for. Currently, only SIGSEGV is
        //                 supported by the runtime.
        // Returns:
        //     NULL if the native bridge doesn't use a handler or doesn't want it to be managed by the
        //     runtime.
        //     Otherwise, a pointer to the signal handler.
        NativeBridgeSignalHandlerFn (*getSignalHandler)(int signal);
    };
};     // namespace android
#endif // NATIVE_BRIDGE_H_