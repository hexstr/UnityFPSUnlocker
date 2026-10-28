package io.github.hexstr.UnityFPSUnlocker;

import android.app.Activity;
import android.content.SharedPreferences;
import android.util.Log;
import android.view.Window;
import android.view.WindowManager;

import androidx.annotation.NonNull;

import java.lang.reflect.Constructor;

import io.github.libxposed.api.XposedModule;

public class MyModule extends XposedModule {
    private static final String tag_ = "UnityFPSUnlocker";

    private int display_mode_id_ = -1;
    private int delay_ = 5;
    private int fps_ = 90;
    private boolean mod_opcode_ = true;
    private float scale_ = -1.0f;

    public static native void HelloWorld(int delay, int fps, boolean mod_opcode, float scale);

    @Override
    public void onPackageReady(@NonNull PackageReadyParam param) {
        String package_name = param.getPackageName();
        ClassLoader class_loader = param.getClassLoader();

        Class<?> unity_player_class;
        Class<?> lifecycle_events_class;
        try {
            unity_player_class = Class.forName("com.unity3d.player.UnityPlayer", false, class_loader);
            lifecycle_events_class = Class.forName("com.unity3d.player.IUnityPlayerLifecycleEvents", false, class_loader);
        } catch (ClassNotFoundException ignored) {
            // Target Unity classes not found in this process; detach to avoid further overhead.
            detach();
            return;
        }

        SharedPreferences settings = null;
        try {
            settings = getRemotePreferences(Prefs.prefs_name_);
        } catch (Throwable t) {
            log(Log.WARN, tag_, "Cannot read remote preferences", t);
        }

        if (settings != null) {
            display_mode_id_ = getIntPref(settings, "display_mode_id", -1);
            delay_ = getIntPref(settings, "delay", 5);
            fps_ = getIntPref(settings, "fps", 90);
            mod_opcode_ = getBooleanPref(settings, "mod_opcode", true);
            scale_ = getFloatPref(settings, "scale", -1.0f);

            display_mode_id_ = getIntPref(
                    settings,
                    package_name + "_per_app_display_mode_id",
                    display_mode_id_);
            delay_ = getIntPref(settings, package_name + "_per_app_delay", delay_);
            fps_ = getIntPref(settings, package_name + "_per_app_fps", fps_);
            mod_opcode_ = getBooleanPref(settings, package_name + "_per_app_mod_opcode", mod_opcode_);
            scale_ = getFloatPref(settings, package_name + "_per_app_scale", scale_);
        } else {
            log(Log.WARN, tag_, "Cannot read settings, using default values");
        }

        boolean is_hooked = false;
        Constructor<?>[] constructors = unity_player_class.getDeclaredConstructors();

        for (Constructor<?> constructor : constructors) {
            Class<?>[] parameter_types = constructor.getParameterTypes();
            boolean match = false;

            for (Class<?> param_type : parameter_types) {
                if (param_type.equals(lifecycle_events_class)) {
                    match = true;
                    break;
                }
            }

            if (match) {
                final int target_display_mode_id = display_mode_id_;
                try {
                    hook(constructor)
                            .setId("unity_player_constructor")
                            .intercept(chain -> {
                                Object result = chain.proceed();
                                Activity activity = null;
                                for (Object arg : chain.getArgs()) {
                                    if (arg instanceof Activity) {
                                        activity = (Activity) arg;
                                        break;
                                    }
                                }

                                if (activity != null) {
                                    if (target_display_mode_id != -1) {
                                        Window window = activity.getWindow();
                                        WindowManager.LayoutParams params = window.getAttributes();
                                        params.preferredDisplayModeId = target_display_mode_id;
                                        window.setAttributes(params);
                                        log(Log.INFO, tag_, "Set display mode to " + target_display_mode_id);
                                    }
                                } else {
                                    log(Log.WARN, tag_, "No activity instance found in arguments.");
                                }
                                return result;
                            });
                    is_hooked = true;
                } catch (Throwable t) {
                    log(Log.ERROR, tag_, "Failed to hook constructor", t);
                }
            }
        }

        if (!is_hooked) {
            log(Log.WARN, tag_, "No target constructors found.");
        }

        log(Log.INFO, tag_, "display_mode_id: " + display_mode_id_ + " | delay: " + delay_ + " | fps: " + fps_ + " | mod_opcode: " + mod_opcode_ + " | scale: " + scale_);
        try {
            System.loadLibrary("UnityFPSUnlocker");
            HelloWorld(delay_, fps_, mod_opcode_, scale_);
        } catch (Throwable t) {
            log(Log.ERROR, tag_, "Failed to load UnityFPSUnlocker native library", t);
        }
    }

    @Override
    public boolean onHotReloading(@NonNull HotReloadingParam param) {
        // Module loads native library; dynamic reload across generations is not supported.
        return false;
    }

    private static int getIntPref(SharedPreferences settings, String key, int fallback) {
        Object val = settings.getAll().get(key);
        if (val instanceof Integer) {
            return (Integer) val;
        } else if (val instanceof String) {
            try {
                return Integer.parseInt((String) val);
            } catch (NumberFormatException ignored) {
            }
        } else if (val instanceof Number) {
            return ((Number) val).intValue();
        }
        return fallback;
    }

    private static float getFloatPref(SharedPreferences settings, String key, float fallback) {
        Object val = settings.getAll().get(key);
        if (val instanceof Float) {
            return (Float) val;
        } else if (val instanceof String) {
            try {
                return Float.parseFloat((String) val);
            } catch (NumberFormatException ignored) {
            }
        } else if (val instanceof Number) {
            return ((Number) val).floatValue();
        }
        return fallback;
    }

    private static boolean getBooleanPref(SharedPreferences settings, String key, boolean fallback) {
        Object val = settings.getAll().get(key);
        if (val instanceof Boolean) {
            return (Boolean) val;
        } else if (val instanceof String) {
            return Boolean.parseBoolean((String) val);
        }
        return fallback;
    }
}
