package io.github.hexstr.UnityFPSUnlocker;

import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;

import androidx.annotation.NonNull;

import io.github.libxposed.service.XposedService;
import io.github.libxposed.service.XposedServiceHelper;

public class Prefs {
    public static final String prefs_name_ = "fps_prefs";
    private static final String tag_ = "UnityFPSUnlocker";

    private static volatile XposedService xposed_service_srv_ = null;
    private static volatile boolean listener_registered_ = false;

    private static final SharedPreferences.OnSharedPreferenceChangeListener pref_change_listener_ =
            (shared_preferences, key) -> syncKeyToRemote(shared_preferences, key);

    public static synchronized void init(Context context) {
        if (!listener_registered_) {
            listener_registered_ = true;
            final Context app_context = context.getApplicationContext();

            SharedPreferences local_prefs = getSharedPrefs(app_context);
            local_prefs.registerOnSharedPreferenceChangeListener(pref_change_listener_);

            XposedServiceHelper.registerListener(new XposedServiceHelper.OnServiceListener() {
                @Override
                public void onServiceBind(@NonNull XposedService service) {
                    xposed_service_srv_ = service;
                    syncAllToRemote(app_context);
                }

                @Override
                public void onServiceDied(@NonNull XposedService service) {
                    if (xposed_service_srv_ == service) {
                        xposed_service_srv_ = null;
                    }
                }
            });
        }
    }

    public static SharedPreferences getSharedPrefs(Context context) {
        return context.getSharedPreferences(prefs_name_, Context.MODE_PRIVATE);
    }

    public static SharedPreferences getRemotePrefs() {
        XposedService service = xposed_service_srv_;
        if (service != null) {
            try {
                return service.getRemotePreferences(prefs_name_);
            } catch (Throwable t) {
                Log.w(tag_, "Failed to get remote preferences", t);
            }
        }
        return null;
    }

    public static void syncAllToRemote(Context context) {
        SharedPreferences remote_prefs = getRemotePrefs();
        if (remote_prefs != null) {
            SharedPreferences local_prefs = getSharedPrefs(context);
            SharedPreferenceCopier.copyAll(local_prefs, remote_prefs, "");
        }
    }

    private static void syncKeyToRemote(SharedPreferences local_prefs, String key) {
        SharedPreferences remote_prefs = getRemotePrefs();
        if (remote_prefs == null || key == null) {
            return;
        }

        Object value = local_prefs.getAll().get(key);
        SharedPreferences.Editor editor = remote_prefs.edit();
        if (value == null) {
            editor.remove(key);
        } else {
            SharedPreferenceCopier.copyValue(editor, key, value);
        }
        editor.apply();
    }
}
