#include "unity_engine.hh"

#include <xdl.h>

#include "utility/config.hh"
#include "utility/logger.hh"

void Unity::Init(void* handle) {
    if ((il2cpp_resolve_icall = (il2cpp_resolve_icall_f)xdl_sym(handle, "il2cpp_resolve_icall", nullptr)) == nullptr) {
        ERROR("il2cpp_resolve_icall not found");
        return;
    }

    if ((set_targetFrameRate = (set_targetFrameRate_f)il2cpp_resolve_icall("UnityEngine.Application::set_targetFrameRate")) == nullptr) {
        ERROR("set_targetFrameRate not found");
    }

    if ((get_currentResolution = (get_currentResolution_t)il2cpp_resolve_icall("UnityEngine.Screen::get_currentResolution_Injected")) == nullptr) {
        ERROR("get_currentResolution not found");
    }

    if ((SetResolution_Internal = (SetResolution_t)il2cpp_resolve_icall("UnityEngine.Screen::SetResolution")) == nullptr) {
        if ((SetResolution_Internal = (SetResolution_t)il2cpp_resolve_icall("UnityEngine.Screen::SetResolution_Injected")) == nullptr) {
            ERROR("SetResolution not found");

            if (auto get_height_ptr = il2cpp_resolve_icall("UnityEngine.Screen::get_height")) {
                auto ptr = reinterpret_cast<uint32_t*>(get_height_ptr);
                auto opcode = ptr[1];
                if ((opcode & 0xFC000000) == 0x94000000) {
                    auto offset = (int32_t)(opcode << 8) >> 6;
                    auto addr = reinterpret_cast<intptr_t>(get_height_ptr) + 4 + offset;
                    auto GetScreenManager = reinterpret_cast<void* (*)(void)>(addr);
                    screen_manager_ = reinterpret_cast<ScreenManager*>(GetScreenManager());
                }
            }

            if (screen_manager_ == nullptr) {
                ERROR("ScreenManager not found");
            }
        }
    }

    if ((GetSystemExtImpl_Internal = (GetSystemExtImpl_t)il2cpp_resolve_icall("UnityEngine.Display::GetSystemExtImpl")) == nullptr) {
        ERROR("GetSystemExtImpl not found");
    }
}

Resolution Unity::GetSystemExtImpl() {
    Resolution resolution;
    if (GetSystemExtImpl_Internal) {
        GetSystemExtImpl_Internal(nullptr, &resolution.m_Width, &resolution.m_Height);
    }
    return resolution;
}

Resolution Unity::GetResolution() {
    Resolution resolution;
    if (get_currentResolution) {
        get_currentResolution(&resolution);
    }
    return resolution;
}

void Unity::SetResolution(float scale) {
    Resolution resolution = GetSystemExtImpl();
    if (scale > 0 && resolution.m_Width > 0) {
        auto target_width = static_cast<int32_t>(resolution.m_Width * scale);
        auto target_height = static_cast<int32_t>(resolution.m_Height * scale);
        int preferred_refresh_rate = 0;

        if (SetResolution_Internal) {
            LOG("Set resolution: %d x %d", target_width, target_height);
            SetResolution_Internal(target_width, target_height, 1, &preferred_refresh_rate);
            Utility::NopFunc(reinterpret_cast<unsigned char*>(SetResolution_Internal));
        }
        else if (screen_manager_) {
            LOG("Set resolution: %d x %d", target_width, target_height);
            screen_manager_->RequestResolution(target_width, target_height, 1, &preferred_refresh_rate);
        }
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
            auto ptr = reinterpret_cast<uint32_t*>(set_targetFrameRate);
            auto opcode = ptr[0];
            if ((opcode & 0xFC000000) == 0x14000000) {
                auto offset = (int32_t)(opcode << 8) >> 6;
                ptr = reinterpret_cast<uint32_t*>(reinterpret_cast<intptr_t>(set_targetFrameRate) + offset);
            }
            Utility::NopFunc(reinterpret_cast<unsigned char*>(ptr));
        }
    }
}