package io.github.hexstr.UnityFPSUnlocker;

import android.content.SharedPreferences;

import java.util.Map;

final class SharedPreferenceCopier {
    private SharedPreferenceCopier() {
    }

    static void copyAll(
            SharedPreferences source_preferences,
            SharedPreferences destination_preferences,
            String key_prefix) {
        if (source_preferences == null || destination_preferences == null) {
            return;
        }

        SharedPreferences.Editor editor = destination_preferences.edit();
        for (Map.Entry<String, ?> entry : source_preferences.getAll().entrySet()) {
            copyValue(editor, key_prefix + entry.getKey(), entry.getValue());
        }
        editor.apply();
    }

    private static void copyValue(
            SharedPreferences.Editor editor,
            String key,
            Object value) {
        if (value instanceof String) {
            editor.putString(key, (String) value);
        } else if (value instanceof Integer) {
            editor.putInt(key, (Integer) value);
        } else if (value instanceof Boolean) {
            editor.putBoolean(key, (Boolean) value);
        } else if (value instanceof Float) {
            editor.putFloat(key, (Float) value);
        } else if (value instanceof Long) {
            editor.putLong(key, (Long) value);
        }
    }
}
