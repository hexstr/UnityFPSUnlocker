package io.github.hexstr.UnityFPSUnlocker;

import java.io.IOException;

import de.robv.android.xposed.IXposedHookLoadPackage;
import de.robv.android.xposed.XSharedPreferences;
import de.robv.android.xposed.XposedBridge;
import de.robv.android.xposed.callbacks.XC_LoadPackage;

public class MyModule implements IXposedHookLoadPackage {
    private int delay = 5;
    private int fps = 90;
    private boolean mod_opcode = true;
    private float scale = -1;

    private static XSharedPreferences getPref(String path) {
        XSharedPreferences pref = new XSharedPreferences(BuildConfig.APPLICATION_ID, path);
        return pref.getFile().canRead() ? pref : null;
    }

    public static native void HelloWorld(int delay, int fps, boolean mod_opcode, float scale);

    @Override
    public void handleLoadPackage(XC_LoadPackage.LoadPackageParam lpparam) throws ClassNotFoundException, IOException, NoSuchMethodException {
        XSharedPreferences settings = getPref("fps_prefs");
        if (settings != null) {
            delay = Integer.parseInt(settings.getString("delay", "5"));
            fps = Integer.parseInt(settings.getString("fps", "90"));
            mod_opcode = settings.getBoolean("mod_opcode", true);
            scale = Float.parseFloat(settings.getString("scale", "-1"));
        }

        XposedBridge.log("delay: " + delay + " | fps: " + fps + " | mod_opcode: " + mod_opcode + " | scale: " + scale);
        System.loadLibrary("UnityFPSUnlocker");
        HelloWorld(delay, fps, mod_opcode, scale);
    }
}