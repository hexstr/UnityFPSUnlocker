#ifndef HOUDINI_HEADER
#define HOUDINI_HEADER

#if defined(__i386__) || defined(__x86_64__)

#include <absl/status/status.h>
#include <absl/status/statusor.h>

#include "singleton.hh"
#include "utility/singleton.hh"

class Houdini : public Singleton<Houdini> {
    friend class Singleton;

public:
    absl::StatusOr<void*> LoadLibrary(const char* name, int flag);
    absl::Status CallJNI(void* handle, void* vm, void* reserved);
    const char* GetError();

private:
    Houdini();
    int houdini_ver_ = 0;
    void* houdini_itf_ = nullptr;
};

#endif // architecture defined

#endif // houdini.hh