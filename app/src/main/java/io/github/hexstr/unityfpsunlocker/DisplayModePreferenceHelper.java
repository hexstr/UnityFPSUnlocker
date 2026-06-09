package io.github.hexstr.UnityFPSUnlocker;

import android.app.Activity;
import android.content.Context;
import android.os.Build;
import android.view.Display;
import android.view.WindowManager;

import androidx.preference.ListPreference;

import java.util.Locale;

final class DisplayModePreferenceHelper {
    private DisplayModePreferenceHelper() {
    }

    static void populate(Activity activity, ListPreference list_preference) {
        if (activity == null || list_preference == null) {
            return;
        }

        Display display = getDisplay(activity);
        if (display == null) {
            return;
        }

        Display.Mode[] modes = display.getSupportedModes();
        String[] entries = new String[modes.length];
        String[] entry_values = new String[modes.length];

        for (int i = 0; i < modes.length; i++) {
            Display.Mode mode = modes[i];
            entries[i] = String.format(
                    Locale.US,
                    "%dx%d@%.2f",
                    mode.getPhysicalWidth(),
                    mode.getPhysicalHeight(),
                    mode.getRefreshRate());
            entry_values[i] = String.valueOf(mode.getModeId());
        }

        list_preference.setEntries(entries);
        list_preference.setEntryValues(entry_values);
    }

    private static Display getDisplay(Activity activity) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            return activity.getDisplay();
        }

        WindowManager window_manager =
                (WindowManager) activity.getSystemService(Context.WINDOW_SERVICE);
        return window_manager == null ? null : window_manager.getDefaultDisplay();
    }
}
