#include "unity_engine.hh"

#include <xdl.h>

#include "utility/config.hh"
#include "utility/logger.hh"

absl::Status Unity::Init(void* handle) {
    if ((il2cpp_resolve_icall = (il2cpp_resolve_icall_f)xdl_sym(handle, "il2cpp_resolve_icall", nullptr)) == nullptr) {
        return absl::NotFoundError("il2cpp_resolve_icall not fount");
    }

    if ((set_targetFrameRate = (set_targetFrameRate_f)il2cpp_resolve_icall("UnityEngine.Application::set_targetFrameRate")) == nullptr) {
        return absl::NotFoundError("set_targetFrameRate not fount");
    }

    if ((get_currentResolution = (get_currentResolution_t)il2cpp_resolve_icall("UnityEngine.Screen::get_currentResolution_Injected")) == nullptr) {
        return absl::NotFoundError("get_currentResolution not fount");
    }

    if ((SetResolution_internal = (SetResolution_t)il2cpp_resolve_icall("UnityEngine.Screen::SetResolution")) == nullptr) {
        return absl::NotFoundError("SetResolution not fount");
    }

    return absl::OkStatus();
}

Resolution Unity::GetResolution() {
    Resolution resolution;
    if (get_currentResolution) {
        get_currentResolution(&resolution);
    }
    return resolution;
}

void Unity::SetResolution(float scale) {
    Resolution resolution = GetResolution();
    if (SetResolution_internal && scale > 0 && resolution.m_Width > 0) {
        SetResolution_internal(resolution.m_Width * scale, resolution.m_Height * scale, 1, 0);
        Utility::NopFunc(reinterpret_cast<unsigned char*>(SetResolution_internal));
    }
}

void Unity::SetFrameRate(int framerate, bool mod_opcode) {
    Resolution resolution = GetResolution();
    if (set_targetFrameRate) {
        set_targetFrameRate(framerate);
        LOG("Current resolution: %dx%d @%d", resolution.m_Width, resolution.m_Height, resolution.m_RefreshRate);
        if (framerate > resolution.m_RefreshRate) {
            ERROR("The screen refresh rate is lower than target framerate! %d < %d", resolution.m_RefreshRate, framerate);
        }

        if (mod_opcode) {
            Utility::NopFunc(reinterpret_cast<unsigned char*>(set_targetFrameRate));
        }
    }
}