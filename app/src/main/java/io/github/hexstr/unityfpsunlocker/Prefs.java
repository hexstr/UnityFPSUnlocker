package io.github.hexstr.UnityFPSUnlocker;

import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;

public class Prefs {
    public static String prefs_name_ = "fps_prefs";

    public static SharedPreferences getSharedPrefs(Context context) {
        try {
            return context.getSharedPreferences(prefs_name_, Context.MODE_WORLD_READABLE);
        } catch (Throwable t) {
            Log.e("hexstr", t.toString());
            return context.getSharedPreferences(prefs_name_, Context.MODE_PRIVATE);
        }
    }
}
