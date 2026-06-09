package io.github.hexstr.UnityFPSUnlocker;

import android.app.Activity;
import android.content.Context;
import android.util.Log;
import android.view.Window;
import android.view.WindowManager;

import de.robv.android.xposed.IXposedHookLoadPackage;
import de.robv.android.xposed.XC_MethodHook;
import de.robv.android.xposed.XSharedPreferences;
import de.robv.android.xposed.XposedBridge;
import de.robv.android.xposed.XposedHelpers;
import de.robv.android.xposed.callbacks.XC_LoadPackage;

public class MyModule implements IXposedHookLoadPackage {
    private int display_mode_id = -1;
    private int delay = 5;
    private int fps = 90;
    private boolean mod_opcode = true;
    private float scale = -1;

    private static XSharedPreferences getPref(String path) {
        XSharedPreferences pref = new XSharedPreferences(BuildConfig.APPLICATION_ID, path);
        return pref.getFile().canRead() ? pref : null;
    }

    private static int getIntPref(XSharedPreferences settings, String key, int fallback) {
        return parseInt(settings.getString(key, String.valueOf(fallback)), fallback);
    }

    private static float getFloatPref(XSharedPreferences settings, String key, float fallback) {
        return parseFloat(settings.getString(key, String.valueOf(fallback)), fallback);
    }

    private static int parseInt(String value, int fallback) {
        try {
            return Integer.parseInt(value);
        } catch (NumberFormatException e) {
            XposedBridge.log("Invalid integer preference value: " + value);
            return fallback;
        }
    }

    private static float parseFloat(String value, float fallback) {
        try {
            return Float.parseFloat(value);
        } catch (NumberFormatException e) {
            XposedBridge.log("Invalid float preference value: " + value);
            return fallback;
        }
    }

    public static native void HelloWorld(int delay, int fps, boolean mod_opcode, float scale);

    @Override
    public void handleLoadPackage(XC_LoadPackage.LoadPackageParam lpparam) {
        String package_name = lpparam.packageName;
        XSharedPreferences settings = getPref("fps_prefs");
        if (settings != null) {
            display_mode_id = getIntPref(settings, "display_mode_id", -1);
            delay = getIntPref(settings, "delay", 5);
            fps = getIntPref(settings, "fps", 90);
            mod_opcode = settings.getBoolean("mod_opcode", true);
            scale = getFloatPref(settings, "scale", -1);

            display_mode_id = getIntPref(
                    settings,
                    package_name + "_per_app_display_mode_id",
                    display_mode_id);
            delay = getIntPref(settings, package_name + "_per_app_delay", delay);
            fps = getIntPref(settings, package_name + "_per_app_fps", fps);
            mod_opcode = settings.getBoolean(package_name + "_per_app_mod_opcode", mod_opcode);
            scale = getFloatPref(settings, package_name + "_per_app_scale", scale);
        } else {
            XposedBridge.log("Cannot read settings");
        }

        XposedHelpers.findAndHookConstructor(
                "com.unity3d.player.UnityPlayer",
                lpparam.classLoader,
                Context.class,
                XposedHelpers.findClass("com.unity3d.player.IUnityPlayerLifecycleEvents", lpparam.classLoader),
                new XC_MethodHook() {
                    @Override
                    protected void afterHookedMethod(MethodHookParam param) {
                        Object contextObj = param.args[0];
                        if (contextObj instanceof Activity) {
                            Activity activity = (Activity) contextObj;
                            if (activity != null && display_mode_id != -1) {
                                Window window = activity.getWindow();
                                WindowManager.LayoutParams params = window.getAttributes();
                                params.preferredDisplayModeId = display_mode_id;
                                window.setAttributes(params);
                                XposedBridge.log("Set display mode to " + display_mode_id);
                            } else {
                                XposedBridge.log("activity is null.");
                            }
                        } else {
                            XposedBridge.log("contextObj is not activity.");
                        }
                    }
                }
        );

        XposedBridge.log("display_mode_id: " + display_mode_id + " | delay: " + delay + " | fps: " + fps + " | mod_opcode: " + mod_opcode + " | scale: " + scale);
        System.loadLibrary("UnityFPSUnlocker");
        HelloWorld(delay, fps, mod_opcode, scale);
    }
}
