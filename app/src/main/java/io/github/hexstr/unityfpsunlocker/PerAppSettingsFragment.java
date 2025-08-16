package io.github.hexstr.UnityFPSUnlocker;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Build;
import android.os.Bundle;
import android.view.Display;
import android.view.WindowManager;

import androidx.preference.ListPreference;
import androidx.preference.PreferenceFragmentCompat;

import java.util.Map;

public class PerAppSettingsFragment extends PreferenceFragmentCompat {
    SharedPreferences.OnSharedPreferenceChangeListener listener =
            (sharedPreferences, key) -> {
                String packageName = sharedPreferences.getString("package_name", "");
                if (packageName.isEmpty()) {
                    return;
                }

                Map<String, ?> allEntries = sharedPreferences.getAll();
                SharedPreferences.Editor editor = PerAppSettingsActivity.prefs_.edit();

                for (Map.Entry<String, ?> entry : allEntries.entrySet()) {
                    String entry_key = entry.getKey();
                    Object entry_value = entry.getValue();

                    String prefixed_key = packageName + "_" + entry_key;

                    if (entry_value instanceof String) {
                        editor.putString(prefixed_key, (String) entry_value);
                    } else if (entry_value instanceof Integer) {
                        editor.putInt(prefixed_key, (Integer) entry_value);
                    } else if (entry_value instanceof Boolean) {
                        editor.putBoolean(prefixed_key, (Boolean) entry_value);
                    }
                }

                editor.apply();
            };


    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        setPreferencesFromResource(R.xml.per_app_preferences, rootKey);

        ListPreference listPreference = findPreference("display_mode_id");

        Display display = null;

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            display = getActivity().getDisplay();
        } else {
            final WindowManager windowManager = (WindowManager) getActivity().getSystemService(Context.WINDOW_SERVICE);
            display = windowManager.getDefaultDisplay();
        }

        Display.Mode[] modes = display.getSupportedModes();

        String[] entries = new String[modes.length];
        String[] entryValues = new String[modes.length];

        for (int i = 0; i < modes.length; i++) {
            Display.Mode mode = modes[i];
            entries[i] = String.format("%dx%d@%.2f",
                    mode.getPhysicalWidth(),
                    mode.getPhysicalHeight(),
                    mode.getRefreshRate());
            entryValues[i] = String.valueOf(mode.getModeId());
        }

        listPreference.setEntries(entries);
        listPreference.setEntryValues(entryValues);
    }

    @Override
    public void onResume() {
        super.onResume();
        getPreferenceManager().getSharedPreferences().registerOnSharedPreferenceChangeListener(listener);

    }

    @Override
    public void onPause() {
        super.onPause();
        getPreferenceManager().getSharedPreferences().unregisterOnSharedPreferenceChangeListener(listener);
    }
}
